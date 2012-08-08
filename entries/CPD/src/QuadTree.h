/* 
 * File:   QuadTree.h
 * Author: abotea
 *
 * Created on 31 March 2011, 4:28 PM
 */

#ifndef _QUADTREE_H
#define	_QUADTREE_H

#include "MoveTable.h"
#include "AbstractGraph.h"
#include <math.h>

class QuadTree {
public:
    QuadTree();
    QuadTree(const QuadTree& orig);
    virtual ~QuadTree();
    void build(const MoveTable & mt, const AbstractGraph * graph);
    void setAsTrivial(const AbstractGraph * graph);
    int getNrBlocks() const {
        return block_ids.size();
    }
    // TODO: not sure if this is correct
    double getAverageNrChecks() const {
        return log(block_ids.size())/log(2);
    }
private:
    int n;
    vector<int> block_ids;
    bool homogeneous(const MoveTable & mt, const AbstractGraph * graph,
        int first_col, int first_row, int length) const;
    void split(const MoveTable & mt, const AbstractGraph * graph,
        int first_col, int first_row, int length);
};

#endif	/* _QUADTREE_H */

