/* 
 * File:   SlimRectangle.h
 * Author: abotea
 *
 * Created on 25 January 2011, 5:47 PM
 */

#ifndef _SLIMRECTANGLE_H
#define	_SLIMRECTANGLE_H

#include "Tile.h"
#include "globals.h"
#include <iostream>
#include <fstream>

class SlimRectangle {
public:
    SlimRectangle();
    virtual ~SlimRectangle();
    //SlimRectangle(int ulc, int ulr, int brc, int brr)
    //:left_col(ulc), upper_row(ulr), right_col(brc), bottom_row(brr),
    //move(NODIRECTION)
    //{};
    SlimRectangle(int ulc, int ulr, int brc, int brr, int move)
    :left_col(ulc), upper_row(ulr), right_col(brc), bottom_row(brr), move(move)
    {};
    bool contains(const Tile & tile) const {
        assert (false);
        if (tile.col < left_col || tile.col > right_col)
            return false;
        if (tile.row < upper_row || tile.row > bottom_row)
            return false;
        return true;
    };
    int getMove() const {
        return move;
    };
    int getLeftCol() const {
        return left_col;
    }
    int getRightCol() const {
        return right_col;
    }
    int getUpperRow() const {
        return upper_row;
    }
    int getBottomRow() const {
        return bottom_row;
    }
    int getWidth() const {
        return right_col - left_col + 1;
    }
    int getHeight() const {
        return bottom_row - upper_row + 1;
    }
    void setMove(int move) {
        this->move = move;
    }
    void writeDataToStream(ofstream & outfile) const;
    void readDataFromStream(ifstream & infile);
protected:
    int left_col;
    int upper_row;
    int right_col;
    int bottom_row;
    // The move label common to all locations in this rectangle.
    // NODIRECTION value means that either the rectangle contains several
    // types of moves, or that the rectangle hasn't been analyzed yet to check
    // the types of moves.
    int move;
};

#endif	/* _SLIMRECTANGLE_H */

