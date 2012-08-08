/* 
 * File:   SlimmerRectangle.cpp
 * Author: abotea
 * 
 * Created on 9 March 2011, 10:12 PM
 */

#include "SlimmerRectangle.h"

SlimmerRectangle::SlimmerRectangle() {
}

SlimmerRectangle::SlimmerRectangle(const SlimmerRectangle& orig) {
    for (int i = 0; i < 6; i++)
        data[i] = orig.data[i];
}

SlimmerRectangle::~SlimmerRectangle() {
}

void SlimmerRectangle::writeDataToStream(ofstream & outfile) const {
    outfile << this->getLeftCol() << " ";
    outfile << this->getUpperRow() << " ";
    outfile << this->getRightCol() << " ";
    outfile << this->getBottomRow() << " ";
    outfile << this->getMove();
}

void SlimmerRectangle::readDataFromStream(ifstream & infile) {
    int leftcol, rightcol, upperrow, bottomrow, move;
    infile >> leftcol;
    infile >> upperrow;
    infile >> rightcol;
    infile >> bottomrow;
    infile >> move;
    init(leftcol, upperrow, rightcol, bottomrow, move);
}
