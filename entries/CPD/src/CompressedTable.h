/* 
 * File:   CompressedTable.h
 * Author: Abotea
 *
 * Created on April 28, 2010, 12:03 PM
 */

#ifndef _COMPRESSEDTABLE_H
#define	_COMPRESSEDTABLE_H

#include <algorithm>
#include <set>
#include <vector>
#include "globals.h"
#include "Rectangle.h"
#include "Tile.h"
#include "AbstractGraph.h"
#include "SlimmerRectangle.h"
#include <stdlib.h>
#include <string.h>
#include "utils2.h"


using namespace std;

class CompressedTable {
public:
    CompressedTable(int size[4]) {
        for (int i = 0; i < 4; i++) {
            default_moves[i] = 0;
            allocate_mem(size[i], i);
        }
    }
    CompressedTable();
    CompressedTable(const CompressedTable & orig);
    virtual ~CompressedTable();
    CompressedTable & operator= (const CompressedTable & orig) {
        for (int i = 0; i < 4; i++) {
            default_moves[i] = orig.default_moves[i];
            this->nr_records[i] = orig.nr_records[i];
            allocate_mem(orig.nr_records[i], i);
            memcpy(this->data3[i], orig.data3[i], 6*nr_records[i]*sizeof(unsigned char));
        }
        this->covered_area = orig.covered_area;
    }
    //int getMove(const Tile & dest) const;
    int getMove(const Tile & dest, int orign_col, int origin_row, 
    int &nr_rect_checks, const AbstractGraph * graph) const;
    void addSlimmerRectangle(const SlimmerRectangle & rectangle, int idx, int quarter) {
        //cerr << "idx = " << idx << "; nr_records = " << nr_records[quarter] << endl;
        assert (0 <= idx && idx < nr_records[quarter]);
        for (int i = 0; i < 6; i++) {
            data3[quarter][6*idx+i] = rectangle.getData(i);
        }
    }

    void addRectangles(const vector<Rectangle> & elems, int quarter);
    const vector<double> & getCoveredAreaStats() const {
        return this->covered_area;
    }
    int getNrRectangles() const {
        return nr_records[0] + nr_records[1] + nr_records[2] + nr_records[3];
    }
    int getNrRectangles(int quarter) const {
        return nr_records[quarter];
    }
    double getAverageNrChecks() const {
        return this->average_nr_checks;
    }
    double computeAverageNrChecks() const;
    void computeCoveredAreaStats();
    void readDataBinary(ifstream & infile);
    void writeDataBinary(ofstream & outfile) const;
    void readDataFromStream(ifstream & infile);
    void writeDataToStream(ofstream & outfile) const;
    // a trivial compressed table has only one rectangle which covers the
    // whole map
    void setAsTrivial(const AbstractGraph * graph);

    void encodeRecord(int lc, int ur, int rc, int br, int move, int idx, int quarter) {
        for (int i = 0; i < 6; i++)
            data3[quarter][6*idx+i] = 0;
        encodeLC(lc, idx, quarter);
        encodeUR(ur, idx, quarter);
        encodeRC(rc, idx, quarter);
        encodeBR(br, idx, quarter);
        encodeMove(move, idx, quarter);
    }

    unsigned int getMove(int idx, int quarter) const {
        assert (0 <= idx && idx < nr_records[quarter]);
        unsigned int result = (data3[quarter][6*idx+5] & 15);
        assert (result <= 8);
        return result;
    };
    unsigned int getLeftCol(int idx, int quarter) const {
        assert (0 <= idx && idx < this->nr_records[quarter]);
        unsigned int result = (this->data3[quarter][6*idx] & 255) << 3;
        result ^= (this->data3[quarter][6*idx+1] >> 5) & 255;
        assert (0 <= result && result < 2048);
        return result;
    }
    unsigned int getUpperRow(int idx, int quarter) const {
        assert (0 <= idx && idx < nr_records[quarter]);
        unsigned int result = (data3[quarter][6*idx+1] & 31) << 6;
        result ^= (data3[quarter][6*idx+2] >> 2) & 255;
        assert (result >= 0 && result < 2048);
        return result;
    }
    unsigned int getRightCol(int idx, int quarter) const {
        assert (0 <= idx && idx < nr_records[quarter]);
        int result = ((data3[quarter][6*idx+2] & 3) << 9);
        result ^= data3[quarter][6*idx+3] << 1;
        result ^= data3[quarter][6*idx+4] >> 7;
        assert (result >= 0 && result < 2048);
        return result;
    }
    unsigned int getBottomRow(int idx, int quarter) const {
        assert (0 <= idx && idx < nr_records[quarter]);
        unsigned int result = ((data3[quarter][6*idx+4] & 127) << 4);
        result ^= (data3[quarter][6*idx+5] >> 4) & 255;
        assert (0 <= result && result < 2048);
        return result;
    }

    bool contains(const Tile & tile, int idx, int quarter, const AbstractGraph * graph) const;
    
    void setMove(int move, int idx, int quarter) {
        assert (0 <= idx && idx < nr_records[quarter]);
        encodeMove(move, idx, quarter);
    }

    void setDefaultMove(int move, int quarter) {
        default_moves[quarter] = move;
    }

    void setDefaultMoves(int *moves) {
        for (int i = 0; i < 4; i++)
            default_moves[i] = moves[i];
    }

    int getDefaultMove(int target_col, int target_row, int origin_col, int origin_row) const {
        return default_moves[getQuarter(target_col, target_row, origin_col, origin_row)];
    }

    void allocate_mem(int size, int quarter) {
        assert (size >= 0);
        this->nr_records[quarter] = size;
        this->data3[quarter] = new unsigned char[6*size*sizeof(unsigned char)];
    }

private:
    unsigned char* data3[4];
    int nr_records[4];
    // covered_area[i] is the number of nodes
    // contained in the first i rectangles, divided
    // by the total number of reachable nodes
    vector<double> covered_area;
    double average_nr_checks;
    // default move to use when the target is not captured in the current
    // list of rectangles
    // same as default_move, but specific to each of the four
    // "quarters" of the map built around the origin point
    int default_moves[4];

    void encodeLC(int col, int idx, int quarter) {
        assert (0 <= idx && idx < nr_records[quarter]);
        assert (0 <= col && col < 2048); // big nr is 2^11
        assert (sizeof(char) == 1);
        data3[quarter][6*idx] ^= (col >> 3) & 255;
        data3[quarter][6*idx+1] ^= (col << 5) & 255;
    }
    void encodeUR(int row, int idx, int quarter) {
        assert (0 <= idx && idx < nr_records[quarter]);
        assert (0 <= row && row < 2048); // big nr is 2^211
        data3[quarter][6*idx+1] ^= (row >> 6) & 255; // 5 bits go to data[1]
        data3[quarter][6*idx+2] ^= (row << 2) & 255; // remaining 6 bits go to data[2]
    }
    void encodeRC(int col, int idx, int quarter) {
        assert (0 <= idx && idx < nr_records[quarter]);
        assert (0 <= col && col < 2048); // big nr is 2^11
        data3[quarter][6*idx+2] ^= (col >> 9) & 255; // 2 bits go to data[2]
        data3[quarter][6*idx+3] ^= (col >> 1) & 255; // 8 bits go to data[3]
        data3[quarter][6*idx+4] ^= ((col & 1) << 7) & 255; // 1 bit goes to data[4]
    }
    void encodeBR(int row, int idx, int quarter) {
        assert (0 <= idx && idx < nr_records[quarter]);
        assert (0 <= row && row < 2048); // big nr is 2^22
        data3[quarter][6*idx+4] ^= (row >> 4) & 255; // 7 bits go to data[4]
        data3[quarter][6*idx+5] ^= ((row & 15) << 4) & 255; // 4 bits go to data[5]
    }
    void encodeMove(int move, int idx, int quarter) {
        assert (0 <= idx && idx < nr_records[quarter]);
        assert (0 <= move && move <= 8);
        data3[quarter][6*idx+5] ^= (move & 255);
    }
};

#endif	/* _COMPRESSEDTABLE_H */
