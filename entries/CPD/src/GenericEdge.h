/*
 * RoadMapEdge.h
 *
 *  Created on: 08/12/2010
 *      Author: abotea
 */

#ifndef ROADMAPEDGE_H_
#define ROADMAPEDGE_H_

#include <string>
#include "AbstractGraphEdge.h"

using namespace std;

class GenericEdge : public AbstractGraphEdge {
public:
    GenericEdge();

    GenericEdge(int n1, int n2, double d, double t, const string & someid)
    : node1(n1), node2(n2), distance(d), time(t), id(someid) {
    };
    virtual ~GenericEdge();

    int getNode1Idx() const {
        return node1;
    }

    int getNode2Idx() const {
        return node2;
    }

    const string & getId() const {
        return id;
    }

    double getCost() const {
        return time;
    }

    double getDistance() const {
        return distance;
    }

    double getTime() const {
        return time;
    }
private:
    int node1;
    int node2;
    double distance;
    double time;
    string id;
};

#endif /* ROADMAPEDGE_H_ */
