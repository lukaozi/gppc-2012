/* 
 * File:   CompressedTable.cpp
 * Author: Abotea
 * 
 * Created on April 28, 2010, 12:03 PM
 */

#include "CompressedTable.h"
#include "utils.h"

CompressedTable::CompressedTable() {
    for (int i = 0; i < 4; i++) {
        default_moves[i] = 0;
        data3[i] = NULL;
        nr_records[i] = 0;
    }
}

CompressedTable::CompressedTable(const CompressedTable & orig) {
    for (int i = 0; i < 4; i++) {
        default_moves[i] = orig.default_moves[i];
        allocate_mem(orig.nr_records[i], i);
        this->nr_records[i] = orig.nr_records[i];
        memcpy(this->data3[i], orig.data3[i], 6*nr_records[i]*sizeof(unsigned char));
    }
    this->covered_area = orig.covered_area;
}

CompressedTable::~CompressedTable() {
    for (int i = 0; i < 4; i++) {
        if (data3[i] != NULL) {
            assert (nr_records[i] >= 0);
            delete[] data3[i];
            data3[i] = NULL;
            nr_records[i] = 0;
        }
    }
}

void CompressedTable::addRectangles(const vector<Rectangle> & elems, int quarter) {
    int sum = 0;
    double weighted_sum = 0;
    int count = 1;
    int size_one_rects = 0;

    allocate_mem(elems.size(), quarter);

    /*
    multiset<Rectangle> rectangles;
    vector<Rectangle>::const_iterator it;
    for (it = elems.begin(); it != elems.end(); it++) {
        rectangles.insert(*it);
    }*/
    vector<Rectangle>::const_iterator pos;
    for (pos = elems.begin(); pos != elems.end(); pos++) {
        sum += pos->getNrLocations();
        if (pos->getNrLocations() == 1)
            size_one_rects++;
        weighted_sum += count*pos->getNrLocations();
        count++;
        covered_area.push_back(sum);
        SlimmerRectangle r(pos->getLeftCol(), pos->getUpperRow(),
                pos->getRightCol(), pos->getBottomRow(), pos->getMove());
        assert (count - 2 >= 0 && count - 2 < nr_records[quarter]);
        this->addSlimmerRectangle(r, count-2, quarter);
    }
    if (sum == 0) {
        average_nr_checks = 0;
    } else {
        average_nr_checks = weighted_sum/sum;
        for (int i = 0; i < covered_area.size(); i++)
           covered_area[i] /= sum;
    }
}

int CompressedTable::getMove(const Tile & dest, int origin_col, int origin_row,
        int & nr_rect_checks, const AbstractGraph * graph) const {
    int quarter = getQuarter(dest.col, dest.row, origin_col, origin_row);
    nr_rect_checks = 0;
    for (int i = 0; i < nr_records[quarter]; i++) {
        nr_rect_checks++;
        if (contains(dest, i, quarter, graph)) {
            return getMove(i, quarter);
        }
    }
    return getDefaultMove(dest.col, dest.row, origin_col, origin_row);
}

bool CompressedTable::contains(const Tile & tile, int idx, int quarter, 
        const AbstractGraph * graph) const {
    if (tile.col < getLeftCol(idx, quarter) || tile.col > getRightCol(idx, quarter))
        return false;
    if (tile.row < getUpperRow(idx, quarter) || tile.row > getBottomRow(idx, quarter))
        return false;
    return true;
};

void CompressedTable::readDataBinary(ifstream & infile) {
    assert (sizeof (unsigned char) == 1);
    // first read nr records for each sector list
    infile.read((char*) nr_records, 4*sizeof (int));
    // then read the default moves
    infile.read((char*) default_moves, 4*sizeof (int));
    // and now the 4 sector lists
    for (int quarter = 0; quarter < 4; quarter++) {
        assert (data3[quarter] == NULL);
        allocate_mem(nr_records[quarter], quarter);        
        infile.read((char*) data3[quarter], 6*nr_records[quarter]);
        for (int k = 0; k < nr_records[quarter]; k++) {
            assert ((data3[quarter][6*k+5] & 15) <= 8);
            assert ((data3[quarter][6*k+5] & 15) >= 0);
        }
    }    
}


void CompressedTable::writeDataBinary(ofstream & outfile) const {
    outfile.write((char*)nr_records, 4*sizeof(int));
    outfile.write((char*)default_moves, 4*sizeof(int));
    for (int quarter = 0; quarter < 4; quarter++)
        outfile.write((char*) data3[quarter], 6*nr_records[quarter]*sizeof(unsigned char));
}

void CompressedTable::readDataFromStream(ifstream & infile) {
    for (int i = 0; i < 4; i++) {
        infile >> default_moves[i];
    }
    string tmp;
    for (int i = 0; i < 4; i++) {
        infile >> tmp;
        assert (tmp == "LQ");
        infile >> nr_records[i];
        assert (data3[i] == NULL);
        allocate_mem(nr_records[i], i);        
        for (int c = 0; c < nr_records[i]; c++) {
            int leftcol, rightcol, upperrow, bottomrow, move;
            infile >> leftcol;
            infile >> upperrow;
            infile >> rightcol;
            infile >> bottomrow;
            infile >> move;
            encodeRecord(leftcol, upperrow, rightcol, bottomrow, move, c, i);
        }
    }
}

void CompressedTable::writeDataToStream(ofstream & outfile) const {
    for (int q = 0; q < 4; q++)
        outfile << default_moves[q] << " ";
    outfile << endl;
    for (int q = 0; q < 4; q++) {
        outfile << "LQ " << nr_records[q] << endl;
        for (int i = 0; i < nr_records[q]; i++) {
            outfile << this->getLeftCol(i, q) << " ";
            outfile << this->getUpperRow(i, q) << " ";
            outfile << this->getRightCol(i, q) << " ";
            outfile << this->getBottomRow(i, q) << " ";
            outfile << this->getMove(i, q);
            outfile << endl;
        }
    }
}

void CompressedTable::setAsTrivial(const AbstractGraph * graph) {
    this->setDefaultMove(1, 0);
    this->setDefaultMove(1, 1);
    this->setDefaultMove(1, 2);
    this->setDefaultMove(1, 3);
    /*
    Rectangle rectangle(0, 0, graph->getWidth() - 1, graph->getHeight() - 1);
    rectangle.setMove(1); // there is only one move and the counting starts from 1
    vector<Rectangle> rectangles;
    rectangles.push_back(rectangle);
    this->addRectangles(rectangles, 0);
     */
}
