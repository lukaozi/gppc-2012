/* 
 * File:   SlimRectangle.cpp
 * Author: abotea
 * 
 * Created on 25 January 2011, 5:47 PM
 */

#include "SlimRectangle.h"

SlimRectangle::SlimRectangle() {
}

SlimRectangle::~SlimRectangle() {
}

void SlimRectangle::writeDataToStream(ofstream & outfile) const {
    outfile << this->left_col << " ";
    outfile << this->upper_row << " ";
    outfile << this->right_col << " ";
    outfile << this->bottom_row << " ";
    outfile << this->move;
}

void SlimRectangle::readDataFromStream(ifstream & infile) {
    infile >> this->left_col;
    infile >> this->upper_row;
    infile >> this->right_col;
    infile >> this->bottom_row;
    infile >> this->move;
}
