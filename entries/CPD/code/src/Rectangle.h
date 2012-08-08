/* 
 * File:   Rectangle.h
 * Author: Abotea
 *
 * Created on April 13, 2010, 10:06 AM
 */

#ifndef _RECTANGLE_H
#define	_RECTANGLE_H

#include "Tile.h"
#include "SlimRectangle.h"
#include "globals.h"
#include <iostream>
#include <fstream>
#include "AbstractGraph.h"

class Rectangle : public SlimRectangle {
public:
    Rectangle();
    Rectangle(int ulc, int ulr, int brc, int brr)
    :SlimRectangle(ulc, ulr, brc, brr, NODIRECTION), nr_locations(-1)
    {};
    Rectangle(int ulc, int ulr, int brc, int brr, int area, int move)
    :SlimRectangle(ulc, ulr, brc, brr, move), nr_locations(area)
    {};
    virtual ~Rectangle();
    bool operator < (const Rectangle & ref) const {
        if (this->nr_locations > ref.nr_locations)
            return true;
        if (this->nr_locations < ref.nr_locations)
            return false;
        if (this->left_col > ref.left_col)
            return true;
        if (this->left_col < ref.left_col)
            return false;
        if (this->upper_row > ref.upper_row)
            return true;
        if (this->upper_row < ref.upper_row)
            return false;
        if (this->getWidth() > ref.getWidth())
            return true;
        if (this->getWidth() < ref.getWidth())
            return false;
        if (this->getHeight() > ref.getHeight())
            return true;
        return false;
    };
    int getNrLocations() const {
        return nr_locations;
    }
    void setNrLocations(int area) {
        this->nr_locations = area;
    }
    void writeDataToStream(ofstream & outfile) const;
    void readDataFromStream(ifstream & infile);
private:
    // number of nodes contained in this rectangle
    int nr_locations;
};

#endif	/* _RECTANGLE_H */
