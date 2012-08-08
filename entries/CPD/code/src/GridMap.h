/* 
 * File:   GridMap.h
 * Author: Abotea
 *
 * Created on April 12, 2010, 4:27 PM
 */

#ifndef _GRIDMAP_H
#define	_GRIDMAP_H

#include "globals.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <assert.h>
#include "Tile.h"
#include "AbstractGraph.h"
#include "GenericMap.h"

using namespace std;

class GridMap : public GenericMap {
public:
    GridMap();
    GridMap(const GridMap& orig);
    GridMap(const char * fname);
    GridMap(vector<bool> &bits, int w, int h);
    virtual ~GridMap();
    void create(const vector<bool> &bits, int width, int height);
    void loadMap(const char * fname, vector<bool> &bits, int &w, int &h);
    bool isObstacle(int col, int row) const {
        assert (0 <= col && col <= width);
        assert (0 <= row && row <= height);
        return (node_indexing[getIdx(col, row)] == -1);
    }
    vector<bool> getBitMap() const {
        vector<bool> result;
        for (int idx = 0; idx < width*height; idx++) {
            result.push_back(this->node_indexing[idx] != -1);
        }
        return result;
    }

    AbstractGraphNode * getNode2(unsigned int col, unsigned int row) const {
        int id = node_indexing[getIdx(col, row)];
        if (id == -1)
            return NULL;
        assert (this->nodes[id]->getColumn() == col);
        assert (this->nodes[id]->getRow() == row);
        return this->nodes[id];
    }

private:
    int getIdx(int c, int r) const {
        return r * this->width + c;
    }
    // given an index as y*width+x, provide the id of the node
    vector<int> node_indexing;
};

#endif	/* _GRIDMAP_H */
