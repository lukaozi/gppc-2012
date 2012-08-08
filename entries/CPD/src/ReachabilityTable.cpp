/* 
 * File:   ReachabilityTable.cpp
 * Author: abotea
 * 
 * Created on 27 January 2011, 9:25 PM
 */

#include "ReachabilityTable.h"

ReachabilityTable::ReachabilityTable() {
}

ReachabilityTable::ReachabilityTable(const ReachabilityTable& orig) {
}

ReachabilityTable::~ReachabilityTable() {
}

void ReachabilityTable::build(const AbstractGraph * pg) {
    int sccIdx = 0;
    int index = 0; // DFS node number counter
    // initialize indexes and lowlink structures:
    vector<int> indexes;
    vector<int> lowlink;
    vector<bool> stackContents;
    int tmp1, tmp2;
    for (int i = 0; i < pg->getNrNodes(); i++) {
        indexes.push_back(-1);
        lowlink.push_back(-1);
        stackContents.push_back(false);
        scc_labels.push_back(-1);
    }
    vector<AbstractGraphNode*> S; // An empty stack of nodes
    for (int i = 0; i < pg->getNrNodes(); i++) {
        AbstractGraphNode * pn = pg->getNode(i);
        if (pn == NULL)
            continue;
        if (indexes[i] == -1) { // Start a DFS at each node not visited yet
            tarjan(pn, pg, sccIdx, index, S, stackContents, indexes, lowlink);
        }
    }
    // now set values in table
    for (int i = 0; i < sccIdx; i++) {
        vector<bool> elem;
        table.push_back(elem);
        for (int j = 0; j < sccIdx; j++)
            table[i].push_back(false);
    }
    for (int i = 0; i < pg->getNrNodes(); i++) {
        AbstractGraphNode * pn = pg->getNode(i);
        if (pn == NULL)
            continue;
        for (int j = 0; j < pn->getDegree(); j++) {
            const AbstractGraphEdge * pe = pn->getEdge(j);
            AbstractGraphNode * ps = pg->getNode(pn->getNeighborIdx(pe));
            assert (ps != NULL);
            tmp1 = scc_labels[pn->getGlobalIdx()];
            tmp2 = scc_labels[ps->getGlobalIdx()];
            table[tmp1][tmp2] = true;
        }
    }
}

/**
 * Implements Tarjan's algorithm as described on Wikipedia
 * */
void ReachabilityTable::tarjan(AbstractGraphNode *pn,
        const AbstractGraph *pg,
        int & sccIdx,
        int & index, vector<AbstractGraphNode*> & S,
        vector<bool> & stackContents,
        vector<int> & indexes, vector<int> & lowlink) {
    // implement Tarjan's algorithm
    indexes[pn->getGlobalIdx()] = index; // set the depth
    lowlink[pn->getGlobalIdx()] = index;
    index = index + 1;
    S.push_back(pn); // Push v on the stack
    stackContents[pn->getGlobalIdx()] = true;
    for (int i = 0; i < pn->getDegree(); i++) { // Consider successors of v
        const AbstractGraphEdge * pe = pn->getEdge(i);
        AbstractGraphNode *ps = pg->getNode(pn->getNeighborIdx(pe));
        if (indexes[ps->getGlobalIdx()] == -1) { // Was successor v' visited?
            tarjan(ps, pg, sccIdx, index, S, stackContents, indexes, lowlink);  // Recurse
            assert (lowlink[ps->getGlobalIdx()] >= 0);
            assert (lowlink[pn->getGlobalIdx()] >= 0);
            if (lowlink[ps->getGlobalIdx()] < lowlink[pn->getGlobalIdx()])
                lowlink[pn->getGlobalIdx()] = lowlink[ps->getGlobalIdx()];
            // how does the previous stmt work with using -1 for undefined?
        } else if (stackContents[ps->getGlobalIdx()]) {// Was successor v' in stack S?
            assert (indexes[ps->getGlobalIdx()] >= 0);
            assert (lowlink[pn->getGlobalIdx()] >= 0);
            if (indexes[ps->getGlobalIdx()] < lowlink[pn->getGlobalIdx()])
                lowlink[pn->getGlobalIdx()] = indexes[ps->getGlobalIdx()];
        }
    }
    if (lowlink[pn->getGlobalIdx()] == indexes[pn->getGlobalIdx()]) { // Is v the root of an SCC?
        assert (stackContents[pn->getGlobalIdx()]);
        // print "SCC:"
        AbstractGraphNode * ps;
        while (true) {
            assert (S.size() > 0);
            ps = S[S.size() - 1];
            S.pop_back();
            stackContents[ps->getGlobalIdx()] = false;
            scc_labels[ps->getGlobalIdx()] = sccIdx;
            if (ps->getGlobalIdx() == pn->getGlobalIdx())
                break;
        }
        sccIdx++;
    }
}

bool ReachabilityTable::writeToFile(const string & filename) const {
    ofstream outfile;
    cerr << "Opening output file " << filename << endl;
    outfile.open(filename.c_str());
    if (!outfile) {
        cerr << "Unable to open " << filename << endl;
        return false;
    }
    assert (table.size() > 0);
    outfile << table.size() << endl;
    for (int i = 0; i < table.size(); i++) {
        assert (table.size() == table[i].size()); // table is square
        for (int j = 0; j < table[i].size(); j++) {
            outfile << table[i][j] << " ";
        }
        outfile << endl;
    }
    outfile << endl;
    outfile << scc_labels.size() << endl;
    for (int i = 0; i < scc_labels.size(); i++) {
        outfile << scc_labels[i] << endl;
    }
    outfile.close();
    return true; // should implement exceptions instead of return codes
}

bool ReachabilityTable::readFromFile(const string & filename) {
    int table_size;
    bool tmp;
    ifstream infile;
    infile.open(filename.c_str());
    if (!infile) {
        cerr << "Unable to open " << filename << endl;
        return false;
    }
    infile >> table_size;
    table.clear();
    for (int i = 0; i < table_size; i++) {
        vector<bool> elem;
        table.push_back(elem);
        for (int j = 0; j < table_size; j++) {
            infile >> tmp;
            table[i].push_back(tmp);
        }
        assert (table[i].size() == table_size);
    }
    assert (table.size() == table_size);
    // now read scc labels for nodes
    int nr_records;
    infile >> nr_records;
    scc_labels.resize(nr_records);
    int val;
    for (int i = 0; i < nr_records; i++) {
        infile >> val;
        scc_labels[i] = val;
    }
    infile.close();
    return true;
}
