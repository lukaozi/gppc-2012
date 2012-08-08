/* 
 * File:   AbstractCPD.h
 * Author: abotea
 *
 * Created on 29 August 2011, 11:43 AM
 */

#ifndef ABSTRACTCPD_H
#define	ABSTRACTCPD_H

#include "AbstractGraph.h"
#include <vector>

class AbstractCPD {
public:
    AbstractCPD();
    AbstractCPD(const AbstractCPD& orig);
    virtual ~AbstractCPD();

    virtual void readDataFromStream2(ifstream & infile) = 0;
    virtual void findPath(int startIdx, int targetIdx,
            double & cost, int & nr_moves, int & nr_rect_checks,
            const AbstractGraph *graph);
    virtual int getNextLocation(int startIdx, int targetIdx,
        const AbstractGraph * pg);
    /* 
     * When firstrun == TRUE, stop after the first 20 moves
     * (or earlier if the total path has less than 20 moves).
     * When only a partial path is computed, return false.
     * Otherwise, return true.
     */
    virtual bool findPath(int startIdx, int targetIdx, 
    AbstractGraph * pg, vector<int> &path, bool firstrun);
    virtual int getMove(int startIdx, int targetIdx, int & nr_rect_checks,
        const AbstractGraph * graph) = 0;
};

#endif	/* ABSTRACTCPD_H */

