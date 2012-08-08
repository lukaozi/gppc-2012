/* 
 * File:   DijkstraNode.h
 * Author: Abotea
 *
 * Created on April 13, 2010, 12:56 PM
 */

#ifndef _DIJKSTRANODE_H
#define	_DIJKSTRANODE_H

struct DijkstraNode {
    DijkstraNode(int c, int r, double d)
    :idx(0), col(c), row(r), distance(d)
    {}
    DijkstraNode(int id, double d)
    :idx(id), col(-1), row(-1), distance(d)
    {}
    int idx;
    int col;
    int row;
    double distance;
};

class CompareDijkstraNodes {
    public:
    bool operator()(DijkstraNode & t1, DijkstraNode & t2)
    {
       if (t1.distance > t2.distance) return true;
       return false;
    }
};

#endif	/* _DIJKSTRANODE_H */
