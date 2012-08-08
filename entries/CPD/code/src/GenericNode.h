/*
 * RoadMapNode.h
 *
 *  Created on: 08/12/2010
 *      Author: abotea
 */

#ifndef ROADMAPNODE_H_
#define ROADMAPNODE_H_

#include <string>
#include <vector>
#include <assert.h>
#include <iostream>
#include "GenericEdge.h"
#include "AbstractGraphNode.h"
#include "AbstractGraphEdge.h"
#include "globals.h"

using namespace std;

class GenericNode : public AbstractGraphNode {
public:
    GenericNode();

    GenericNode(int idx, double somex, double somey, const string & sid)
    : globalIdx(idx), x(somex), y(somey), string_id(sid) {
        edges.clear();
    };
    GenericNode(int idx, int c, int r)
    : globalIdx(idx), column(c), row(r)
    {
        edges.clear();
    }
    GenericNode(const GenericNode & n);
    virtual ~GenericNode();

    int getColumn() const {
        return this->column;
    }

    int getRow() const {
        return this->row;
    }

    int getNeighborIdx(const AbstractGraphEdge * edge) const {
        return edge->getNode2Idx();
    }

    void setColumn(int c) {
        column = c;
    }

    void setRow(int r) {
        row = r;
    }

    const AbstractGraphEdge* getEdge(unsigned int index) const {
        assert(0 <= index && index < edges.size());
        return edges[index];
    }

    void addEdge(AbstractGraphEdge * pe) {
        assert(edges.size() < MAX_NODE_DEGREE - 1);
        edges.push_back(pe);
    }

    const vector<AbstractGraphEdge*> & getEdges() const {
        return edges;
    }

    int getGlobalIdx() const {
        return globalIdx;
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

    int getDegree() const {
        return edges.size();
    }

    const string & getStringId() const {
        return string_id;
    }

private:
    int globalIdx;
    double x;
    double y;
    string string_id;
    // coordinates in a grid
    int column;
    int row;
    // edges to neighbors
    vector<AbstractGraphEdge*> edges;
};

#endif /* ROADMAPNODE_H_ */
