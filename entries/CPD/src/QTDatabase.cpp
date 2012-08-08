/* 
 * File:   QTDatabase.cpp
 * Author: abotea
 * 
 * Created on 1 April 2011, 11:47 AM
 */

#include "QTDatabase.h"

QTDatabase::QTDatabase() {
}

QTDatabase::QTDatabase(const QTDatabase& orig) {
}

QTDatabase::~QTDatabase() {
}

void QTDatabase::build() {
    try {
        time_t starttime = time(NULL);
        this->nr_blocks = 0;
        this->average_checks = 0.0;
        int tmp = 0;
        assert(last_node_idx < pg->getNrNodes());
        assert(first_node_idx >= 0);
        assert(last_node_idx >= first_node_idx);
        this->qts.resize(last_node_idx - first_node_idx + 1);
        for (int it = first_node_idx; it <= last_node_idx; it++) {
            const AbstractGraphNode * pn = pg->getNode(it);
            if (pn == NULL)
                continue;
            int col = pn->getColumn();
            int row = pn->getRow();
            QuadTree qt;
            if (pn->getDegree() == 1) {
                // build quickly a trivial quadtree with only one square;
                // no need for expensive processing in this case
                qt.setAsTrivial(pg);
            } else {
                MoveTable table(col, row, it);
                table.computeTable(pg);
                qt.build(table, pg);
            }
            this->addQuadTree(it - first_node_idx, qt);
            this->nr_blocks += qt.getNrBlocks();
            double checks = qt.getAverageNrChecks();
            if (checks != 0) {
                this->average_checks += checks;
                tmp++;
            }
            if (false && it % 1000 == 0) {
                time_t currenttime = time(NULL);
                time_t diff = currenttime - starttime;
                cerr << "at iteration " << it << " after "
                        << diff << " seconds" << endl;
                cerr << "[" << col << ", " << row << "]: ";
                cerr << qt.getNrBlocks() << " qt blocks built for this node";
                cerr << endl;
                // cerr << "nr of edges of this node: " << pn->getDegree() << endl;
                // cerr << ct << endl << endl;
            }
        }
        this->average_checks /= tmp;
    } catch (exception& e) {
        cerr << "exception in build: " << e.what() << endl;
        exit(3);
    }
}
