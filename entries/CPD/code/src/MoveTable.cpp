/* 
 * File:   MoveTable.cpp
 * Author: Abotea
 * 
 * Created on April 13, 2010, 9:44 AM
 */

#include <limits>
#include <queue>
#include <deque>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include "MoveTable.h"
#include "Tile.h"
#include "DijkstraNode.h"
#include "utils.h"
#include "RectangleSplitter.h"
#include "AbstractGraphNode.h"
#include "AbstractGraph.h"

using namespace std;

MoveTable::MoveTable() {
}

MoveTable::MoveTable(const MoveTable& orig) {
}

MoveTable::~MoveTable() {
}

void MoveTable::computeTable(const AbstractGraph * graph) {
    vector<bool> already_expanded;
    for (int i = 0; i < graph->getNrNodes(); i++) {
        dist_table2.push_back(std::numeric_limits<int>::max());
        move_table2.push_back(NODIRECTION); // i.e., 0
        already_expanded.push_back(false);
        //orientation.push_back(std::numeric_limits<double>::max());
    }
    priority_queue<DijkstraNode, vector<DijkstraNode>, CompareDijkstraNodes> pq;
    // create root node and push it to queue
    dist_table2[this->orig_node_idx] = 0;
    move_table2[this->orig_node_idx] = NODIRECTION;
    pq.push(DijkstraNode(this->orig_node_idx, 0));
    int nr_expanded = 0;
    // process nodes one by one
    while (!pq.empty()) {
        DijkstraNode node = pq.top();
        double dist = dist_table2[node.idx];
        if (dist == std::numeric_limits<int>::max()) {
            break;
        }
        pq.pop();
        if (already_expanded[node.idx])
            continue;
        already_expanded[node.idx] = true;
        // expand node
        const AbstractGraphNode* an = graph->getNode(node.idx);
        const vector<AbstractGraphEdge*> & edges = an->getEdges();
        int moveidx = 0;
        nr_expanded++;
        for (vector<AbstractGraphEdge*>::const_iterator it = edges.begin(); it != edges.end(); it++) {
            moveidx++;
            int neighborIdx = an->getNeighborIdx(*it);
            if (already_expanded[neighborIdx])
                continue;
            double cost = (*it)->getCost();
            double alt = dist + cost;
            assert(dist < std::numeric_limits<int>::max());
            if (should_mark_as_parent(graph, node.idx, neighborIdx, alt, moveidx)) {
                DijkstraNode n(neighborIdx, alt);
                pq.push(n);
                dist_table2[neighborIdx] = alt;
                if (node.idx == this->orig_node_idx) {
                    move_table2[neighborIdx] = moveidx;
                    assert(moveidx > 0);
                } else {
                    move_table2[neighborIdx] = move_table2[node.idx];
                    assert(move_table2[neighborIdx] > 0);
                }
                //orientation[neighborIdx] = this->compute_orientation(graph, node.idx, neighborIdx);
            }
        }
    }
}

bool MoveTable::should_mark_as_parent(const AbstractGraph *graph,
        int nodeIdx, int neighborIdx, double new_dist, int new_move) const {
    if (new_dist < dist_table2[neighborIdx]) {
        return true;
    }
    //return false;
    // FOR GRIDS ONLY -- UGLY HACK BECAUSE WE DON'T KNOW THE EFFECTS
    // ON ROAD MAPS
    if (new_dist == dist_table2[neighborIdx]) {
        AbstractGraphNode *nod = graph->getNode(nodeIdx);
        AbstractGraphNode *nei = graph->getNode(neighborIdx);
        if (nod->getColumn() == nei->getColumn())
            return true;
        if (nod->getRow() == nei->getRow())
            return true;
    }
    // END OF <FOR GRIDS ONLY>
    return false; // don't remove this unless experiments prove it's worthwhile
    //const AbstractGraphNode *an = graph->getNode(neighborIdx);
    //const AbstractGraphNode *on = graph->getNode(this->orig_node_idx);
    if ((dist_table2[neighborIdx] == new_dist)) {
        if (move_table2[neighborIdx] < new_move)
        //if (orientation[neighborIdx] > this->compute_orientation(graph, nodeIdx, neighborIdx))
        //move_table2[nodeIdx] == graph->calculateDefaultMove(on, an));
        //cerr << "modifying existing parent" << endl;
            return true;
    }
    return false;
}

double MoveTable::compute_orientation(const AbstractGraph *graph,
            int nodeIdx1, int nodeIdx2) const {
    const AbstractGraphNode *n1 = graph->getNode(nodeIdx1);
    const AbstractGraphNode *n2 = graph->getNode(nodeIdx2);
    double v1 = (n1->getX() - n2->getX())/(n1->getY() - n2->getY());
    if (v1 < 0)
        v1 = -v1;
    return min(v1, 1/v1);
}

void MoveTable::computeDominantMoves(const AbstractGraph * graph) {
    int tmp[4][MAX_NODE_DEGREE + 1];
    for (int quarter = 0; quarter < 4; quarter++)
        for (int j = 0; j <= MAX_NODE_DEGREE; j++)
            tmp[quarter][j] = 0;
    for (int i = 0; i < graph->getNrNodes(); i++) {
        int quarter = -1;
        int col = graph->getNode(i)->getColumn();
        int row = graph->getNode(i)->getRow();
        if (col <= orig_col && row <= orig_row) {
            quarter = 0;
        } else if (col > orig_col && row <= orig_row) {
            quarter = 1;
        } else if (col > orig_col && row > orig_row) {
            quarter = 2;
        } else {
            quarter = 3;
        }
        tmp[quarter][this->move_table2[i]]++;
    }
    for (int quarter = 0; quarter < 4; quarter++) {
        int max = -1;
        for (int j = 0; j <= MAX_NODE_DEGREE; j++) {
            if (tmp[quarter][j] > max) {
                max = tmp[quarter][j];
                dominant_moves[quarter] = j;
            }
        }
    }
}
