#include <vector>
#include <assert.h>
#include "Entry.h"
#include "CompressedMoveDatabase.h"
#include <stdio.h>
#include "GridMap.h"
#include "MapCpd.h"
#include <pthread.h>


void * build_cpd_function(void* data) {
    CompressedMoveDatabase *pc = (CompressedMoveDatabase *) data;
    pc->build(true);
}

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{
    CompressedMoveDatabase * cpds[8];
    AbstractGraph *pg = new GridMap(bits, width, height);
    pthread_t thread[8];
    int iret[8];
    int nr_nodes = pg->getNrNodes();
    int increment = nr_nodes/8 + 1;
    int startidx = 0;
    for (int i = 0; i < 8; i++) {
        int endidx = startidx + increment;
        if (endidx >= nr_nodes)
            endidx = nr_nodes - 1;
        cerr << "Thread " << i << " computing CPD from node ";
        cerr << startidx << " to node " << endidx << endl;
        cpds[i] = new CompressedMoveDatabase(pg, startidx, endidx);
        startidx = endidx + 1;
        iret[i] = pthread_create( &thread[i], NULL, 
                build_cpd_function, (void*) cpds[i]);
    }
    for (int i = 0; i < 8; i++) {
        pthread_join(thread[i], NULL);
    }

    ofstream outfile;
    outfile.open(filename, ios::binary);
    if (!outfile) {
        cerr << "Unable to open " << filename << endl;
        for (int i = 0; i < 8; i++) {
            delete cpds[i];
        }
        exit (1);
    }

    outfile.write((char*) (&nr_nodes) , sizeof(int));
    for (int i = 0; i < 8; i++) {
        cpds[i]->writeDataBinaryNoHeader(outfile);
        delete cpds[i];
    }
    outfile.close();
}

void PreprocessMap2(std::vector<bool> &bits, int width, int height, const char *filename)
{
    AbstractGraph *pg = new GridMap(bits, width, height);
    CompressedMoveDatabase cmd(pg, 0, pg->getNrNodes()-1);
    cmd.build(true);
    ofstream outfile;
    outfile.open(filename, ios::binary);
    if (!outfile) {
        cerr << "Unable to open " << filename << endl;
        delete pg;
        exit (1);
    }
    cmd.writeDataBinary(outfile);
    delete pg;
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
    MapCpd * mapcpd = new MapCpd(bits, w, h, filename);
    return (void*) mapcpd;
}


#ifdef MOVEBYMOVEPATH
/*
 * Returns one move at a time, to provide more accurate statistics
 * for the max time per move, which apparently will be collected
 * in the competition.
 */
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path) {
    MapCpd * mapcpd = (MapCpd*) data;
    GridMap * pg = mapcpd->getGraph();
    CompressedMoveDatabase * cpd = mapcpd->getCPD();
    // if this is the first call, add s to path
    if (path.size() == 0) {
        path.push_back(s);
    }
    // if s == g, nothing left to do
    if (s.x == g.x && s.y == g.y)
        return true;

    xyLoc sl = path[path.size()-1];
    int startIdx = pg->getNode2(sl.x, sl.y)->getGlobalIdx();
    int targetIdx = pg->getNode2(g.x, g.y)->getGlobalIdx();
    int nextIdx = cpd->getNextLocation(startIdx, targetIdx, pg);
    xyLoc nl;
    nl.x = pg->getNode(nextIdx)->getColumn();
    nl.y = pg->getNode(nextIdx)->getRow();
    path.push_back(nl);
    return (nextIdx == targetIdx);
}


#else

/* 
 * Implementation that first returns the first 20 moves,
 * and then the rest of the moves (if any left)
 */
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path) {
    MapCpd * mapcpd = (MapCpd*) data;
    GridMap * pg = mapcpd->getGraph();
    CompressedMoveDatabase * cpd = mapcpd->getCPD();
    vector<int> intpath;
    int targetIdx = pg->getNode2(g.x, g.y)->getGlobalIdx();
    bool result;
    if (path.size() == 0) {
        int startIdx = pg->getNode2(s.x, s.y)->getGlobalIdx();
        result = cpd->findPath(startIdx, targetIdx, pg, intpath, true);
    } else {
        xyLoc l = path[path.size()-1];
        int startIdx = pg->getNode2(l.x, l.y)->getGlobalIdx();
        result = cpd->findPath(startIdx, targetIdx, pg, intpath, false);
    }
    for (int k = 0; k < intpath.size(); k++) {
        xyLoc l;
        l.x = pg->getNode(intpath[k])->getColumn();
        l.y = pg->getNode(intpath[k])->getRow();
        path.push_back(l);
    }
    return result;
}

#endif


const char *GetName() {
    return "CPD";
}