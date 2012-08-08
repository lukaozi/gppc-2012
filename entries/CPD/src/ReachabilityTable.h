/* 
 * File:   ReachabilityTable.h
 * Author: abotea
 *
 * Created on 27 January 2011, 9:25 PM
 */

#ifndef REACHABILITYTABLE_H
#define	REACHABILITYTABLE_H

#include <vector>
#include <assert.h>
#include <string>
#include <stdlib.h>
#include "AbstractGraph.h"
#include "AbstractGraphNode.h"
#include <iostream>
#include <fstream>


using namespace std;

class ReachabilityTable {
public:
    ReachabilityTable();
    ReachabilityTable(const ReachabilityTable& orig);
    virtual ~ReachabilityTable();
    bool pathExists(const AbstractGraphNode *pn1, const AbstractGraphNode *pn2) const {
        return pathExists(pn1->getGlobalIdx(), pn2->getGlobalIdx());
    }
    bool pathExists(int nodeIdx1, int nodeIdx2) const {
        int v1 = scc_labels[nodeIdx1];
        int v2 = scc_labels[nodeIdx2];
        assert (0 <= v1 && v1 < table.size());
        assert (0 <= v2 && v2 < table[v1].size());
        return table[v1][v2];
    }
    // implements Tarjan's algorithm
    void build(const AbstractGraph * pg);
    bool writeToFile(const string & filename) const;
    bool readFromFile(const string & filename);
private:
    void tarjan(AbstractGraphNode *pn, const AbstractGraph* pg,
            int & sccIdx,
            int & index, vector<AbstractGraphNode*> & S,
            vector<bool> & stackContents,
            vector<int> & indexes, vector<int> & lowlink);
    vector< vector<bool> > table;
    vector<int> scc_labels;
};

#endif	/* REACHABILITYTABLE_H */

