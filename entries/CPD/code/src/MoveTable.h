/* 
 * File:   MoveTable.h
 * Author: Abotea
 *
 * Created on April 13, 2010, 9:44 AM
 */

#ifndef _MOVETABLE_H
#define	_MOVETABLE_H

#include "globals.h"
#include "CompressedTable.h"
#include "GridMap.h"
#include "AbstractGraph.h"
#include <iostream>

using namespace std;

class MoveTable {
public:
    MoveTable();
    MoveTable(const MoveTable& orig);

    MoveTable(int col, int row)
    : orig_col(col), orig_row(row) {
    }

    MoveTable(int col, int row, int id)
    : orig_col(col), orig_row(row), orig_node_idx(id) {
    }
    virtual ~MoveTable();
    void computeTable(const AbstractGraph * graph);

    int getMove(int idx) const {
        //cerr << "idx = " << idx << "; size = " << move_table2.size() << endl;
        assert(0 <= idx && idx < move_table2.size());
        return move_table2[idx];
    }

    int getOriginIdx() const {
        return orig_node_idx;
    }

    int getOrigCol() const {
        return orig_col;
    }

    int getOrigRow() const {
        return orig_row;
    }

    void computeDominantMoves(const AbstractGraph* graph);
    int getDominantMove(int quarter) const {
        assert (0 <= quarter && quarter < 4);
        return dominant_moves[quarter];
    }

private:
    // data
    int orig_col;
    int orig_row;

    // new data structures to work with more abstracted graph representation
    vector<int> move_table2;
    vector<double> dist_table2;
    int orig_node_idx;

    // which move appears the most in each of the map quarters around orig_node
    int dominant_moves[4];
    // orientation of the line between parent and current node
    // defined as min(|tg|,|ctg|)
    //vector<double> orientation;
    bool should_mark_as_parent(const AbstractGraph *graph,
            int nodeIdx, int neighborIdx, double new_dist, int new_move) const;
    double compute_orientation(const AbstractGraph *graph,
            int nodeIdx1, int nodeIdx2) const;
};

#endif	/* _MOVETABLE_H */
