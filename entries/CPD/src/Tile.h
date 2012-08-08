/* 
 * File:   Tile.h
 * Author: Abotea
 *
 * Created on April 13, 2010, 11:40 AM
 */

#ifndef _TILE_H
#define	_TILE_H

#include <assert.h>
#include "globals.h"
#include <iostream>

using namespace std;

class Tile {
public:
    Tile();
    Tile(const Tile& orig);
    Tile(int c, int r, int index)
    :col(c), row(r), id(index)
    {
        //cerr << "Tile(" << col << "," << row << "; " << id << ")" << endl;
    }
    virtual ~Tile();
    int col;
    int row;
    int id;
private:

};

#endif	/* _TILE_H */

