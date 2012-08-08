/*
 * RoadMapNode.cpp
 *
 *  Created on: 08/12/2010
 *      Author: abotea
 */

#include "GenericNode.h"

GenericNode::GenericNode() {
    edges.clear();
}

GenericNode::GenericNode(const GenericNode & n) {
    this->column = n.column;
    this->row = n.row;
    this->globalIdx = n.globalIdx;
    edges.clear();
}

GenericNode::~GenericNode() {
    for (vector<AbstractGraphEdge*>::iterator it = edges.begin(); it != edges.end(); it++)
        delete *it;
}
