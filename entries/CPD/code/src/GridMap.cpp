/* 
 * File:   GridMap.cpp
 * Author: Abotea
 * 
 * Created on April 12, 2010, 4:27 PM
 */

#include "GridMap.h"
#include <iostream>
#include <fstream>
#include <limits>

GridMap::GridMap() {
}

GridMap::GridMap(const GridMap& orig) {
    assert (false);
}

GridMap::~GridMap() {
}

GridMap::GridMap(const char * fname) {
	vector<bool> bits;
        int w, h;
	loadMap(fname, bits, w, h);
	create(bits, w, h);
}

GridMap::GridMap(vector<bool> &bits, int w, int h) {
	this->create(bits, w, h);
}

void GridMap::loadMap(const char *fname, vector<bool> &map, int &w, int &h) {
    FILE *f;
    f = fopen(fname, "r");
    if (f) {
        fscanf(f, "type octile\nheight %d\nwidth %d\nmap\n", &h, &w);
        map.resize(h*w);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                char c;
                if (x == w - 1)
                    fscanf(f, "%c\n", &c);
                else 
                    fscanf(f, "%c", &c);
                map[y*w+x] = (c == '.' || c == 'G' || c == 'S');
            }
        }
        fclose(f);
    }
}

void GridMap::create(const vector<bool> &bits, int width, int height) {
    int id = 0;
    this->width = width;
    this->height = height;
    this->node_indexing.resize(width*height);
    // add nodes
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y*width+x;
            if (bits[index]) {
                assert (nodes.size() == id);
                AbstractGraphNode * p = new GenericNode(id, x, y);
                nodes.push_back(p);
                node_indexing[index] = id;
                assert (nodes[id]->getColumn() == x);
                assert (nodes[id]->getRow() == y);
                assert (id < height*width);
                id++;
            } else {
                node_indexing[index] = -1;
            }
        }
    }
    // add edges
    for (int id = 0; id < this->getNrNodes(); id++) {
        AbstractGraphNode * pn = this->getNode(id);
        int col = pn->getColumn();
        int row = pn->getRow();
        for (int r = -1; r <= 1; r++) {
            for (int c = -1; c <= 1; c++) {
                if (r == 0 && c == 0)
                    continue;
                if (col + c < 0 || col + c >= width)
                    continue;
                if (row + r < 0 || row + r >= height)
                    continue;
                AbstractGraphNode * pneigh = this->getNode2(col + c, row + r);
                if (pneigh == NULL)
                    continue;
                int neigh_id = pneigh->getGlobalIdx();
                if (c == 0 || r == 0) {
                    AbstractGraphEdge *pe = new GenericEdge(id, neigh_id, ONE, ONE, "");
                    nodes[id]->addEdge(pe);
                } else {
                    AbstractGraphNode * pdiag1 = this->getNode2(col, row + r);
                    if (pdiag1 == NULL)
                        continue;
                    AbstractGraphNode * pdiag2 = this->getNode2(col + c, row);
                    if (pdiag2 == NULL)
                        continue;
                    AbstractGraphEdge *pe = new GenericEdge(id, neigh_id, SQRT2, SQRT2, "");
                    nodes[id]->addEdge(pe);
                }
            }
        }
    }
    // build the indexing for rows and columns
    indexNodesOnRowsAndCols();
}
