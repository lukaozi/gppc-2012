/*
 * AbstractGraphEdge.h
 *
 *  Created on: 10/12/2010
 *      Author: abotea
 */

#ifndef ABSTRACTGRAPHEDGE_H_
#define ABSTRACTGRAPHEDGE_H_

class AbstractGraphEdge {
public:
    AbstractGraphEdge();
    virtual ~AbstractGraphEdge();
    virtual double getCost() const = 0;
    virtual int getNode2Idx() const = 0;
};

#endif /* ABSTRACTGRAPHEDGE_H_ */
