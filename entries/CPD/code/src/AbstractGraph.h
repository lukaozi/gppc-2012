/*
 * AbstractGraph.h
 *
 *  Created on: 10/12/2010
 *      Author: abotea
 */

#ifndef ABSTRACTGRAPH_H_
#define ABSTRACTGRAPH_H_

#include <vector>
#include "AbstractGraphNode.h"

using namespace std;

class AbstractGraph {
public:
	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
	virtual int getNrNodes() const = 0;
	virtual AbstractGraphNode* getNode(unsigned int idx) const = 0;
	//virtual const AbstractGraphNode* getNode(int col, int row) const = 0;
	virtual const vector<AbstractGraphNode*> & getNodes() const = 0;
	virtual const vector<int> & getNodeIndexesOnColumn(unsigned int c) const = 0;
	virtual const vector<int> & getNodeIndexesOnRow(unsigned int r) const = 0;
        virtual int calculateDefaultMove(const AbstractGraphNode *origin, const AbstractGraphNode *dest) const = 0;
};

#endif /* ABSTRACTGRAPH_H_ */
