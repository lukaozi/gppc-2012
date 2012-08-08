/* 
 * File:   SlimmerRectangle.h
 * Author: abotea
 *
 * Created on 9 March 2011, 10:12 PM
 */

#ifndef SLIMMERRECTANGLE_H
#define	SLIMMERRECTANGLE_H

#include "Tile.h"
#include "globals.h"
#include <iostream>
#include <fstream>

class SlimmerRectangle {
public:
    SlimmerRectangle();
    SlimmerRectangle(const SlimmerRectangle& orig);
    virtual ~SlimmerRectangle();
    //SlimmerRectangle(int lc, int ur, int rc, int br)
    //{
    //    init(lc, ur, rc, br, 0);
    //};
    SlimmerRectangle(int lc, int ur, int rc, int br, int move)
    {
        init(lc, ur, rc, br, move);
    };
    
    int getMove() const {
        unsigned int result = (data[5] & 15);
        assert (0<= result && result < 8);
        return result;
    };
    unsigned int getLeftCol() const {
        unsigned int result = (this->data[0] & 255) << 3;
        result ^= (this->data[1] >> 5) & 255;
        assert (0 <= result && result < 2048);
        return result;
    }
    int getRightCol() const {
        int result = ((data[2] & 3) << 9);
        result ^= data[3] << 1;
        result ^= data[4] >> 7;
        assert (result >= 0 && result < 2048);
        return result;
    }
    int getUpperRow() const {
        unsigned int result = (data[1] & 31) << 6;
        result ^= (data[2] >> 2) & 255;
        assert (result >= 0 && result < 2048);
        return result;
    }
    int getBottomRow() const {
        unsigned int result = ((data[4] & 127) << 4);
        result ^= (data[5] >> 4) & 255;
        assert (0 <= result && result < 2048);
        return result;
    }
    bool contains(const Tile & tile) const {
        if (tile.col < getLeftCol() || tile.col > getRightCol())
            return false;
        if (tile.row < getUpperRow() || tile.row > getBottomRow())
            return false;
        return true;
    };
    int getWidth() const {
        return getRightCol() - getLeftCol();
    }
    int getHeight() const {
        return getBottomRow() - getUpperRow();
    }
    void setMove(int move) {
        encodeMove(move);
    }
    unsigned char getData(int idx) const {
        assert (0 <= idx && idx < 6);
        return data[idx];
    }
    void writeDataToStream(ofstream & outfile) const;
    void readDataFromStream(ifstream & infile);
protected:
    // the 6 bytes of these three data members will
    // be used to store the corner coordinates
    // (i.e., 2 columns and 2 rows) and
    // one move label:
    // first 11 bits are for left column index
    // next 11 bits are for upper row index
    // next 11 bits are for right column index
    // next 11 bits are for bottom row index
    // remaining 4 bits are for move label
    unsigned char data[6];
private:
    void encodeLC(int col) {
        assert (0 <= col && col < 2048); // big nr is 2^11
        assert (sizeof(char) == 1);
        data[0] ^= (col >> 3) & 255;
        data[1] ^= (col << 5) & 255;
    }
    void encodeUR(int row) {
        assert (0 <= row && row < 2048); // big nr is 2^211
        data[1] ^= (row >> 6) & 255; // 5 bits go to data[1]
        data[2] ^= (row << 2) & 255; // remaining 6 bits go to data[2]
    }
    void encodeRC(int col) {
        assert (0 <= col && col < 2048); // big nr is 2^11
        data[2] ^= (col >> 9) & 255; // 2 bits go to data[2]
        data[3] ^= (col >> 1) & 255; // 8 bits go to data[3]
        data[4] ^= ((col & 1) << 7) & 255; // 1 bit goes to data[4]
    }
    void encodeBR(int row) {
        assert (0 <= row && row < 2048); // big nr is 2^22
        data[4] ^= (row >> 4) & 255; // 7 bits go to data[4]
        data[5] ^= ((row & 15) << 4) & 255; // 4 bits go to data[5]
    }
    void encodeMove(int move) {
        assert (0 <= move && move <= 8);
        data[5] ^= (move & 15);
    }
    void init(int lc, int ur, int rc, int br, int move) {
        assert (sizeof(data[0]) == 1);
        for (int i = 0; i < 6; i++)
            data[i] = 0;
        encodeLC(lc);
        encodeUR(ur);
        encodeRC(rc);
        encodeBR(br);
        encodeMove(move);
    }
};

#endif	/* SLIMMERRECTANGLE_H */
