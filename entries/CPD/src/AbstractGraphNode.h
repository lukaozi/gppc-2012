/*
 * AbstractGraphNode.h
 *
 *  Created on: 10/12/2010
 *      Author: abotea
 */

#ifndef ABSTRACTGRAPHNODE_H_
#define ABSTRACTGRAPHNODE_H_

#include <vector>
#include <string>
#include "AbstractGraphEdge.h"
#include <assert.h>

using namespace std;

class AbstractGraphNode {
public:
    AbstractGraphNode();
    virtual ~AbstractGraphNode();
    virtual int getColumn() const = 0;
    virtual int getRow() const = 0;
    virtual double getX() const = 0;
    virtual double getY() const = 0;
    virtual void setRow(int row) = 0;
    virtual void setColumn(int column) = 0;
    virtual const vector<AbstractGraphEdge*> & getEdges() const = 0;
    virtual const AbstractGraphEdge * getEdge(unsigned int index) const = 0;
    virtual int getDegree() const = 0;
    virtual int getNeighborIdx(const AbstractGraphEdge * edge) const = 0;
    virtual int getGlobalIdx() const = 0;
    virtual const string & getStringId() const = 0;
    virtual void addEdge(AbstractGraphEdge *pe) = 0;

    bool operator<(const AbstractGraphNode & ref) const {
        if (this->getColumn() < ref.getColumn())
            return true;
        if (this->getColumn() > ref.getColumn())
            return false;
        if (this->getRow() < ref.getRow())
            return true;
        if (this->getRow() > ref.getRow())
            return false;
        return true;
    };

};

#endif /* ABSTRACTGRAPHNODE_H_ */
