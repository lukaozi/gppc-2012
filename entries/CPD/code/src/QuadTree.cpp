/* 
 * File:   QuadTree.cpp
 * Author: abotea
 * 
 * Created on 31 March 2011, 4:28 PM
 */

#include "QuadTree.h"
#include "AbstractGraph.h"
#include <math.h>

QuadTree::QuadTree() {
}

QuadTree::QuadTree(const QuadTree& orig) {
}

QuadTree::~QuadTree() {
}

void QuadTree::build(const MoveTable & mt, const AbstractGraph * graph) {
    double max_dim = max(graph->getWidth(), graph->getHeight());
    double log_2_max = log(max_dim)/log(2);
    this->n = (int) ceil(log_2_max);
    int length = (int) pow(2, n);
    split(mt, graph, 0, 0, length);
}

void QuadTree::setAsTrivial(const AbstractGraph * graph) {
    double max_dim = max(graph->getWidth(), graph->getHeight());
    double log_2_max = log(max_dim)/log(2);
    this->n = (int) ceil(log_2_max);
    this->block_ids.push_back(1); // TODO: but real value instead of 1
}


// TODO: debug.
bool QuadTree::homogeneous(const MoveTable & mt, const AbstractGraph * graph,
        int first_col, int first_row, int length) const {
    assert (length >= 1);
    int move_label = -1;
    for (int c = first_col; c < first_col + length; c++) {
        if (c >= graph->getWidth())
            break;
        const vector<int> & nodes = graph->getNodeIndexesOnColumn(c);
        for (unsigned int it = 0; it < nodes.size(); it++) {
            const AbstractGraphNode * pn = graph->getNode(nodes[it]);
            if (pn->getRow() >= first_row + length || pn->getRow() < first_row)
                continue;
            if (pn->getRow() >= graph->getHeight())
                break; // we are outside the map
            int new_move_label = mt.getMove(nodes[it]);
            assert(new_move_label >= 0);
            if (new_move_label > 0 && move_label != new_move_label && move_label > 0) {
                assert (length > 1); // 1x1 squares must be homogeneous
                return false;
            } else if (new_move_label > 0)
                move_label = new_move_label;
        }
    }
    return true;
}

void QuadTree::split(const MoveTable & mt, const AbstractGraph * graph,
        int first_col, int first_row, int length) {
    if (homogeneous(mt, graph, first_col, first_row, length)) {
        this->block_ids.push_back(1); // TODO: need a proper value, not 1
    } else {
        //cerr << "splitting " << first_col << ", " << first_row << ", " << length << endl;
        split(mt, graph, first_col, first_row, length/2); // upper left
        split(mt, graph, first_col + length/2, first_row, length/2); // upper right
        split(mt, graph, first_col, first_row + length/2, length/2); // lower left
        split(mt, graph, first_col + length/2, first_row + length/2, length/2); // lower right
    }
}
