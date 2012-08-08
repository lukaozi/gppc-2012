/* 
 * File:   QTDatabase.h
 * Author: abotea
 *
 * Created on 1 April 2011, 11:47 AM
 */

#ifndef _QTDATABASE_H
#define	_QTDATABASE_H

#include <vector>
#include "QuadTree.h"

using namespace std;

class QTDatabase {
public:
    QTDatabase();
    QTDatabase(const QTDatabase& orig);
    QTDatabase(AbstractGraph * pg, int first, int last)
    : pg(pg), first_node_idx(first), last_node_idx(last) {
    }
    QTDatabase(AbstractGraph * pg)
    : pg(pg), first_node_idx(0), last_node_idx(pg->getNrNodes() - 1) {
    }
    virtual ~QTDatabase();
    void build();
    int getNrBlocks() const {
        return this->nr_blocks;
    }
    double getNrAverageChecks() const {
        return this->average_checks;
    }
private:
    AbstractGraph *pg;
    int first_node_idx;
    int last_node_idx;
    vector<QuadTree> qts;
    // total number of blocks in all quadtrees
    int nr_blocks;
    // Average number of checks to find the right rectangle in a list
    double average_checks;

    void addQuadTree(int idx, const QuadTree & qt) {
        assert(0 <= idx && idx < qts.size());
        this->qts[idx] = qt;
    }
};

#endif	/* _QTDATABASE_H */

