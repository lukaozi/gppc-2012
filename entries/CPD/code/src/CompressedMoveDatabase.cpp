/* 
 * File:   CompressedMoveDatabase.cpp
 * Author: Abotea
 * 
 * Created on April 13, 2010, 9:54 AM
 */

#include "CompressedMoveDatabase.h"
#include "MoveTable.h"
#include "utils.h"
#include "SlimmerRectangle.h"
#include "TableCompresser1.h"
//#include "TableCompresser2.h"

CompressedMoveDatabase::CompressedMoveDatabase() {
}

CompressedMoveDatabase::CompressedMoveDatabase(const CompressedMoveDatabase& orig) {
}

CompressedMoveDatabase::~CompressedMoveDatabase() {
}

/**
 * Builds a compressed path database
 */
void CompressedMoveDatabase::build(bool list_trimming) {
    try {
        int max = -1;
        TableCompresser1 tc;
        ////TableCompresser2 tc;
        time_t starttime = time(NULL);
        this->nr_rectangles = 0;
        this->average_checks = 0.0;
        int nr_compressed_tables = 0;
        assert(last_node_idx < pg->getNrNodes());
        assert(first_node_idx >= 0);
        assert(last_node_idx >= first_node_idx);
        //tables2.reserve(last_node_idx - first_node_idx + 1);
        this->tables2.resize(last_node_idx - first_node_idx + 1);
        for (int it = first_node_idx; it <= last_node_idx; it++) {
            const AbstractGraphNode * pn = pg->getNode(it);
            if (pn == NULL)
                continue;
            int col = pn->getColumn();
            int row = pn->getRow();
            CompressedTable ct;
            if (pn->getDegree() == 1) {
                // build quickly a trivial compressed table with only
                // one rectangle; no need for expensive processing in this case
                ct.setAsTrivial(pg);
                nr_rectangles_before_demo += 1;
            } else {
                MoveTable table(col, row, it);
                table.computeTable(pg);
                ct = tc.compressTable(col, row, table, pg, list_trimming);
                nr_1x1_rectangles += tc.getNr1x1Rectangles();
                nr_rectangles_before_demo += tc.getNrRectanglesBeforeDeMoFiltering();
            }
            this->addCompressedTable(it - first_node_idx, ct);
            this->nr_rectangles += ct.getNrRectangles();
            double checks = ct.getAverageNrChecks();
            if (checks != 0) {
                this->average_checks += checks;
                nr_compressed_tables++;
            }
            if (ct.getNrRectangles() > max)
                max = ct.getNrRectangles();
            if (it % 100 == 0 && false) {
                time_t currenttime = time(NULL);
                time_t diff = currenttime - starttime;
                cerr << "at iteration " << it << " after "
                        << diff << " seconds" << endl;
                cerr << nr_rectangles << " rectangles built so far" << endl;
                cerr << "[" << col << ", " << row << "]: ";
                cerr << ct.getNrRectangles() << " rectangles built for this node" << endl;
                cerr << "max size of a list of rectangles so far: " << max << endl;
                if (it > first_node_idx)
                    cerr << "avg size of a list of rectangles so far: "
                         << 1.0*nr_rectangles/(it - first_node_idx) << endl;
                cerr << endl;
                //cerr << "could eliminate " << tc.counter << " rects by merging" << endl;
                // cerr << "nr of edges of this node: " << pn->getDegree() << endl;
                // cerr << ct << endl << endl;
            }
        }
        this->average_checks /= nr_compressed_tables;
        computeCoveredAreaStats(pg);
    } catch (exception& e) {
        cerr << "exception in build: " << e.what() << endl;
        exit(3);
    }
}

void CompressedMoveDatabase::computeCoveredAreaStats(const AbstractGraph * graph) {
    vector<int> counter;
    for (int idx = first_node_idx; idx <= last_node_idx; idx++) {
        const CompressedTable & ct = this->tables2[idx - first_node_idx];
        const vector<double> & stats = ct.getCoveredAreaStats();
        for (unsigned int i = 0; i < stats.size(); i++) {
            if (i >= this->covered_area_stats.size()) {
                this->covered_area_stats.push_back(stats[i]);
            } else {
                this->covered_area_stats[i] += stats[i];
            }
            if (i >= counter.size()) {
                counter.push_back(1);
                assert(counter[i] == 1);
            } else {
                counter[i]++;
            }
        }
    }
    for (unsigned int i = 0; i < this->covered_area_stats.size(); i++) {
        this->covered_area_stats[i] /= counter[i];
    }
}

void CompressedMoveDatabase::readDataFromStream2(ifstream & infile) {
    int nr_records;
    int default_moves[4], nr_rectangles[4];
    int idx;
    double xc, yc;
    string lh_symbol;
    infile >> nr_records;
    this->tables2.resize(nr_records);
    for (int rec = 0; rec < nr_records; rec++) {
        infile >> lh_symbol;
        infile >> idx;
        infile >> xc;
        infile >> yc;
        infile >> xc;
        infile >> yc;
        for (int i = 0; i < 4; i++) {
            infile >> default_moves[i];
        }
        CompressedTable ct;
        ct.setDefaultMoves(default_moves);
        this->tables2[idx] = ct;
        //tables2[idx].setDefaultMoves(default_moves);
        assert(idx == rec);
        string tmp;
        for (int i = 0; i < 4; i++) {
            infile >> tmp;
            assert (tmp == "LQ");
            infile >> nr_rectangles[i]; // size of list of rectangles for each quarter
            if (nr_rectangles[i] > 0)
                this->tables2[idx].allocate_mem(nr_rectangles[i], i);
            for (int x = 0; x < nr_rectangles[i]; x++) {
                int sc, sr, tc, tr, move;
                infile >> sc;
                infile >> sr;
                infile >> tc;
                infile >> tr;
                infile >> move;
                SlimmerRectangle rectangle(sc, sr, tc, tr, move);
                this->tables2[idx].addSlimmerRectangle(rectangle, x, i);
                assert (sc == tables2[idx].getLeftCol(x, i));
                assert (sr == tables2[idx].getUpperRow(x, i));
                assert (tc == tables2[idx].getRightCol(x, i));
                assert (tr == tables2[idx].getBottomRow(x, i));
                assert (move == tables2[idx].getMove(x, i));
            }
        }
    }
}

void CompressedMoveDatabase::writeDataToStream(ofstream & outfile) const {
    outfile << tables2.size() << endl << endl;
    for (int it = 0; it < tables2.size(); it++) {
        int node_idx = it + first_node_idx;
        const AbstractGraphNode * pn = pg->getNode(node_idx);
        if (pn == NULL)
            continue;
        outfile << "LH ";
        outfile << pn->getGlobalIdx() << " " << pn->getX() << " " << pn->getY();
        outfile << " " << pn->getColumn() << " " << pn->getRow() << endl;
        // TODO: add here default move
        (this->tables2[it]).writeDataToStream(outfile);
        outfile << endl;
    }
    outfile.close();
}

void CompressedMoveDatabase::readDataBinary(ifstream & infile) {
    // read default moves
    int nr_nodes;
    infile.read((char*) &nr_nodes, sizeof (int));
    this->tables2.resize(nr_nodes);
    for (int rec = 0; rec < nr_nodes; rec++) {
        CompressedTable ct;
        ct.readDataBinary(infile);
        this->tables2[rec] = ct;
    }
    infile.close();
}

void CompressedMoveDatabase::writeDataBinary(ofstream & outfile) const {
    int n = this->tables2.size();
    outfile.write((char*) (&n) , sizeof(int));
    for (int i = 0; i < this->tables2.size(); i++)
        tables2[i].writeDataBinary(outfile);
}

void CompressedMoveDatabase::writeDataBinaryNoHeader(ofstream & outfile) const {
    for (int i = 0; i < this->tables2.size(); i++)
        tables2[i].writeDataBinary(outfile);    
}