/*
 * RoadMap.h
 *
 *  Created on: 08/12/2010
 *      Author: abotea
 */

#ifndef ROADMAP_H_
#define ROADMAP_H_

#include <string>
#include <vector>
#include <tr1/unordered_map>
#include "GenericNode.h"
#include "GenericEdge.h"
#include "AbstractGraph.h"
#include "AbstractGraphNode.h"

using namespace std;
using namespace tr1;

class GenericMap : public AbstractGraph {
public:
    GenericMap();
    GenericMap(std::vector<bool> &bits, int w, int h);
    GenericMap(const string & vertexfilename, const string & edgefilename);
    virtual ~GenericMap();

    // inherited from AbstractGraph:
    int getWidth() const;
    int getHeight() const;
    int getNrNodes() const;
    const vector<int> & getNodeIndexesOnColumn(unsigned int c) const;
    const vector<int> & getNodeIndexesOnRow(unsigned int r) const;
    const vector<AbstractGraphNode*> & getNodes() const;

    AbstractGraphNode* getNode(unsigned int idx) const {
        assert(0 <= idx && idx < nodes.size());
        return nodes[idx];
    }
    
    int calculateDefaultMove(const AbstractGraphNode *origin, const AbstractGraphNode *dest) const;

protected:
    vector<AbstractGraphNode*> nodes;
    vector<vector<int> > node_indexing_per_columns;
    vector<vector<int> > node_indexing_per_rows;
    //vector<vector<int> > node_indexes;
    int width;
    int height;
    void readNodes(const string & filename, unordered_map<string, int> & index_map);
    void readEdges(const string & filename, unordered_map<string, int> & index_map);
    void computeGrid();
    void setNodeColumns();
    void setNodeRows();
    void indexNodesOnRowsAndCols();
};

#endif /* ROADMAP_H_ */
