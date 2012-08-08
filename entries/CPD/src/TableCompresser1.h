/* 
 * File:   TableCompresser1.h
 * Author: abotea
 *
 * Created on 27 April 2011, 5:33 PM
 */

#ifndef _TABLECOMPRESSER1_H
#define	_TABLECOMPRESSER1_H

#include "TableCompresser.h"

class TableCompresser1 : public TableCompresser {
public:
    TableCompresser1();
    TableCompresser1(const TableCompresser1& orig);
    virtual ~TableCompresser1();
    CompressedTable compressTable(int col, int row,
            const MoveTable & mt, const AbstractGraph * graph, bool list_trimming);
    int getNrRectanglesBeforeDeMoFiltering() const {
        return nr_rectangles_before_demo;
    };
    int getNr1x1Rectangles() const {
        return nr_1x1_rectangles;
    }    
private:
    int counter;
    int nr_rectangles_before_demo;
    int nr_1x1_rectangles;
    int mergeRectsInList(vector<Rectangle> & rectangles);
    CompressedTable compressTable1(int col, int row,
            const MoveTable & mt, const AbstractGraph * graph);
    CompressedTable compressTable2(int col, int row,
            const MoveTable & mt, const AbstractGraph * graph, bool list_trimming);
    CompressedTable compressTable9(int col, int row,
            const MoveTable & mt, const AbstractGraph * graph, bool list_trimming);
    bool rectangle_encodes_default_moves(const MoveTable & mt,
            const AbstractGraph * graph, const Rectangle & r) const;
    vector<Rectangle> filter_rectangles_by_default_move1(vector<Rectangle> & rectangles,
        const MoveTable & mt, const AbstractGraph * graph) const;
    void filter_rectangles_by_default_move2(multiset<Rectangle> & input,
    vector<Rectangle> & filtered_rectangles, int & default_move) const;
    void filter_rectangles_by_default_move3(vector<Rectangle> & rectangles,
        const MoveTable & mt, const AbstractGraph * graph,
        vector<Rectangle> & filtered_rectangles, int * default_moves) const;
    bool skip_rectangle(int position_in_list, int list_size, int nr_locations) const;
    int detect_quarter(const Rectangle &r, const MoveTable &mt) const;
    Rectangle shrinkTo1x1(const Rectangle & r, 
        const AbstractGraph * graph, const MoveTable & mt) const;
    void processRectangles(vector<Rectangle> & rectangles, vector<Rectangle> & fr,
        int & default_move, const MoveTable & mt, const AbstractGraph * graph,
            bool list_trimming);
};

#endif	/* _TABLECOMPRESSER1_H */

