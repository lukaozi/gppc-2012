/* 
 * File:   Rectangle.cpp
 * Author: Abotea
 * 
 * Created on April 13, 2010, 10:06 AM
 */

#include "Rectangle.h"
#include "utils.h"

Rectangle::Rectangle() {
}

Rectangle::~Rectangle() {
}

void Rectangle::writeDataToStream(ofstream & outfile) const {
    outfile << this->nr_locations << " ";
    outfile << this->left_col << " ";
    outfile << this->upper_row << " ";
    outfile << this->right_col << " ";
    outfile << this->bottom_row << " ";
    outfile << this->move;
}

void Rectangle::readDataFromStream(ifstream & infile) {
    infile >> this->nr_locations;
    infile >> this->left_col;
    infile >> this->upper_row;
    infile >> this->right_col;
    infile >> this->bottom_row;
    infile >> this->move;
}

/*
void Rectangle::shrinkTo1x1(const AbstractGraph * graph, const MoveTable & mt) {
    assert (this->nr_locations == 1);
    for (int c = left_col; c <= right_col; c++) {
        const vector<int> & nodes = graph->getNodeIndexesOnColumn(c);
        for (unsigned int it = 0; it < nodes.size(); it++) {
            const AbstractGraphNode * pn = graph->getNode(nodes[it]);
            if (pn == NULL)
                continue;
            if (pn->getRow() > this->getBottomRow())
                continue;
            if (pn->getRow() < this->getUpperRow())
                continue;
            if (mt.getMove(pn->getGlobalIdx()) == 0)
                continue; // this is an unreachable node
            // now we reached the only location contained in this rectangle
            left_col = pn->getColumn();
            upper_row = pn->getRow();
            right_col = 0;
            bottom_row = 0;
            return;
        }
    }
}
*/
