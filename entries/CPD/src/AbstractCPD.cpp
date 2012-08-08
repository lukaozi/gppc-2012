/* 
 * File:   AbstractCPD.cpp
 * Author: abotea
 * 
 * Created on 29 August 2011, 11:43 AM
 */

#include "AbstractCPD.h"
#include <iostream>

using namespace std;

AbstractCPD::AbstractCPD() {
}

AbstractCPD::AbstractCPD(const AbstractCPD& orig) {
}

AbstractCPD::~AbstractCPD() {
}

void AbstractCPD::findPath(int startIdx, int targetIdx,
            double & cost, int & nr_moves, int & nr_rect_checks,
            const AbstractGraph *graph) {
    cost = 0.0;
    nr_moves = 0;
    nr_rect_checks = 0;
    int currentIdx = startIdx;
    int nr = 0;
    while (currentIdx != targetIdx) {
        int move = this->getMove(currentIdx, targetIdx, nr, graph) - 1;
        assert (move >= -1);
        //cerr << "move is " << move << endl;
        nr_rect_checks += nr;
        if (move < 0) {
            // there is no path from current node to target
            cost = -1.0;
            return;
        }
        nr_moves++;
        const AbstractGraphEdge * edge = graph->getNode(currentIdx)->getEdge(move);
        cost += edge->getCost();
        //cerr << "[" << currentIdx << ", ";
        //cerr << graph->getNode(currentIdx)->getColumn() << ", ";
        //cerr << graph->getNode(currentIdx)->getRow() << "] ---- ";
        //cerr << "(" << move << ", " << edge->getCost() << ") ---- ";
        currentIdx = edge->getNode2Idx();
        //cerr << "[" << currentIdx << ", ";
        //cerr << graph->getNode(currentIdx)->getColumn() << ", ";
        //cerr << graph->getNode(currentIdx)->getRow() << "]" << endl ;
    }
}

int AbstractCPD::getNextLocation(int startIdx, int targetIdx,
        const AbstractGraph * pg) {
    int nr;
    int move = this->getMove(startIdx, targetIdx, nr, pg) - 1;
    assert (move >= 0);
    const AbstractGraphEdge * edge = pg->getNode(startIdx)->getEdge(move);
    assert (edge != NULL);
    return edge->getNode2Idx();
}

bool AbstractCPD::findPath(int startIdx, int targetIdx, 
        AbstractGraph * pg, vector<int> &path, bool firstrun) {
    int nr_moves = 0;
    int currentIdx = startIdx;
    int nr = 0;
    if (firstrun)
        path.push_back(startIdx);
    while (currentIdx != targetIdx) {
        int move = this->getMove(currentIdx, targetIdx, nr, pg) - 1;
        assert (move >= -1);
        if (move < 0) {
            // there is no path from current node to target
            return false;
        }
        const AbstractGraphEdge * edge = pg->getNode(currentIdx)->getEdge(move);
        currentIdx = edge->getNode2Idx();
        path.push_back(currentIdx);
        nr_moves++;
        if (firstrun && nr_moves == 20)
            return false;
    }
    return true;
}
