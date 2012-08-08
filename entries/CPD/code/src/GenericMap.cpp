/*
 * RoadMap.cpp
 *
 *  Created on: 08/12/2010
 *      Author: abotea
 */

#include "utils.h"
#include "GenericMap.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <math.h>
#include <limits>
#include "utils.h"

GenericMap::GenericMap() {
}


GenericMap::~GenericMap() {
    for (vector<AbstractGraphNode*>::iterator it = nodes.begin(); it != nodes.end(); it++)
        delete *it;
}

GenericMap::GenericMap(const string & vertexfilename, const string & edgefilename) {
    unordered_map<string, int> index_map;
    readNodes(vertexfilename, index_map);
    readEdges(edgefilename, index_map);
    computeGrid();
}

void GenericMap::readNodes(const string & filename, unordered_map<string, int> & index_map) {
    int globalId = 0;
    string token1;
    double token2, token3;
    ifstream infile;
    cerr << "Opening input file " << filename.c_str() << endl;
    infile.open(filename.c_str());
    if (!infile) {
        cerr << "Unable to open " << filename << endl;
        exit (1);
    }
    infile >> token1; // read keyword "ID"
    //cerr << token << endl;
    assert (token1 == "ID");
    infile >> token1; // read keyword "LON"
    //cerr << token << endl;
    assert (token1 == "LON");
    infile >> token1; // read keyword "LAT"
    //cerr << token << endl;
    assert (token1 == "LAT");

    nodes.clear();
    while (infile >> token1 >> token2 >> token3) {
        if (globalId > 0 && token2 == nodes[globalId-1]->getX() &&
                token3 == nodes[globalId-1]->getY()) {
            cerr << "at line " << globalId << endl;
            cerr << "value: " << token2 << endl;
            cerr << "error in data: two vertices with identical coordinates" << endl;
            exit(2);
        }
        AbstractGraphNode * p = new GenericNode(globalId, token2, token3, token1);
        nodes.push_back(p);
        index_map[token1] = globalId;
        globalId++;
    }
    cerr << "done reading vertices" << endl;
    infile.close();
}

void GenericMap::readEdges(const string & filename, unordered_map<string, int> & index_map) {
    string token, token1, token2, token5, token6;
    float token3, token4;
    ifstream infile;
    cerr << "Opening input file " << filename.c_str() << endl;
    infile.open(filename.c_str());
    if (!infile) {
        cerr << "Unable to open " << filename << endl;
        exit (1);
    }

    infile >> token; // read keyword "F_JNCTID"
    //cerr << token << endl;
    assert (token == "F_JNCTID");
    infile >> token; // read keyword "T_JNCTID"
    //cerr << token << endl;
    assert (token == "T_JNCTID");
    infile >> token; // read keyword "METERS"
    //cerr << token << endl;
    assert (token == "METERS");
    infile >> token; // read keyword "MINS"
    //cerr << token << endl;
    assert (token == "MINS");
    infile >> token; // read keyword "ONEWAY"
    //cerr << token << endl;
    assert (token == "ONEWAY");
    infile >> token; // read keyword "ID"
    //cerr << token << endl;
    assert (token == "ID");

    bool onewayflag;
    while (!infile.eof()) {
        infile >> token1;
        infile >> token2;
        infile >> token3;
        infile >> token4;
        infile >> token5;
        infile >> token6;
        onewayflag = !(token5 == "F");
        //cerr << "parsing line " << token1 << " " << token2 << " " << token3 << " " << token4 << " " << token5 << " " << token6 << endl;
        // add edge to one or both corresponding nodes
        if (index_map.find(token1) != index_map.end() && index_map.find(token2) != index_map.end()) {
            int idx1 = index_map[token1];
            int idx2 = index_map[token2];
            AbstractGraphEdge *pe = new GenericEdge(idx1, idx2, token3, token4, token6);
            nodes[idx1]->addEdge(pe);
            if (!onewayflag) {
                AbstractGraphEdge *pe = new GenericEdge(idx2, idx1, token3, token4, token6);
                nodes[idx2]->addEdge(pe);
            }
        }
    }
    cerr << "done reading edges" << endl;
    infile.close();
}

void GenericMap::computeGrid() {
    setNodeColumns();
    setNodeRows();
    indexNodesOnRowsAndCols();
}

void GenericMap::setNodeColumns() {
    unordered_map<double, int> mymap;
    set<double> uniquexcoords;
    for (vector<AbstractGraphNode*>::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
        uniquexcoords.insert((*it)->getX());
    }
    vector<double> ordereduxc;
    for (set<double>::iterator it = uniquexcoords.begin(); it != uniquexcoords.end(); it++) {
        ordereduxc.push_back(*it);
    }
    sort(ordereduxc.begin(), ordereduxc.end());
    int position = 0;
    for (vector<double>::iterator it = ordereduxc.begin(); it != ordereduxc.end(); it++) {
        mymap[*it] = position;
        //cerr << "x coord: " << setprecision(10) << *it << "; position: " << position << endl;
        position++;
    }
    for (vector<AbstractGraphNode*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
        (*it)->setColumn(mymap[(*it)->getX()]);
    }
    width = position;
}

void GenericMap::setNodeRows() {
    unordered_map<double, int> mymap;
    set<double> uniqueycoords;
    unordered_map<string, GenericNode> nodes_map;
    for (vector<AbstractGraphNode*>::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
        uniqueycoords.insert((*it)->getY());
    }
    vector<double> ordereduyc;
    for (set<double>::iterator it = uniqueycoords.begin(); it != uniqueycoords.end(); it++) {
        ordereduyc.push_back(*it);
    }
    sort(ordereduyc.begin(), ordereduyc.end());
    int position = 0;
    for (vector<double>::iterator it = ordereduyc.begin(); it != ordereduyc.end(); it++) {
        mymap[*it] = position;
        //cerr << "y coord: " << setprecision(10) << *it << "; position: " << position << endl;
        position++;
    }
    for (vector<AbstractGraphNode*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
        (*it)->setRow(mymap[(*it)->getY()]);
    }

    height = position;
}

void GenericMap::indexNodesOnRowsAndCols() {
    bool(*fn_pt)(AbstractGraphNode*, AbstractGraphNode*) = comp;
    multiset<AbstractGraphNode*, bool(*)(AbstractGraphNode*, AbstractGraphNode*) > ordered_nodes(fn_pt);
    vector<AbstractGraphNode*>::const_iterator vit;
    for (int i = 0; i <= height; i++) {
        vector<int> v;
        node_indexing_per_rows.push_back(v);
    }
    for (int i = 0; i <= width; i++) {
        vector<int> v;
        node_indexing_per_columns.push_back(v);
    }
    for (vit = nodes.begin(); vit != nodes.end(); vit++) {
        ordered_nodes.insert(*vit);
    }
    multiset<AbstractGraphNode*>::const_iterator mit, oldmit;
    oldmit = ordered_nodes.begin();
    for (mit = ordered_nodes.begin(); mit != ordered_nodes.end(); mit++) {
        if (mit != oldmit) {
            if ((*oldmit)->getColumn() == (*mit)->getColumn())
                assert ((*oldmit)->getRow() <= ((*mit))->getRow());
            if ((*oldmit)->getRow() == (*mit)->getRow()) 
                assert ((*oldmit)->getColumn() <= ((*mit))->getColumn());
        }
        node_indexing_per_rows[(*mit)->getRow()].push_back((*mit)->getGlobalIdx());
        node_indexing_per_columns[(*mit)->getColumn()].push_back((*mit)->getGlobalIdx());
        oldmit = mit;
    }
}

int GenericMap::getWidth() const {
    return this->width;
}

int GenericMap::getHeight() const {
    return this->height;
}

int GenericMap::getNrNodes() const {
    return nodes.size();
}

const vector<AbstractGraphNode*> & GenericMap::getNodes() const {
    return this->nodes;
}

const vector<int> & GenericMap::getNodeIndexesOnColumn(unsigned int c) const {
    return this->node_indexing_per_columns[c];
}

const vector<int> & GenericMap::getNodeIndexesOnRow(unsigned int r) const {
    return this->node_indexing_per_rows[r];
}

int GenericMap::calculateDefaultMove(const AbstractGraphNode *origin, const AbstractGraphNode *dest) const {
    int result = -1;
    double minheur = std::numeric_limits<int>::max();
    const vector<AbstractGraphEdge*> & edges = origin->getEdges();
    int moveidx = 0;
    assert (edges.size() > 0);
    vector<AbstractGraphEdge*>::const_iterator it;
    for (it = edges.begin(); it != edges.end(); it++) {
        moveidx++; // important that the first index is 1
        int neighborIdx = origin->getNeighborIdx(*it);
        const AbstractGraphNode* nn = getNode(neighborIdx);
        double dy = nn->getY() - dest->getY();
        double dx = nn->getX() - dest->getX();
        double heur = sqrt(dy*dy + dx*dx) + (*it)->getCost();
        if (minheur > heur) {
            minheur = heur;
            result = moveidx;
        }
    }
    assert (result != -1);
    return result;
}
