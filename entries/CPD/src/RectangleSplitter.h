/* 
 * File:   RectangleSplitter.h
 * Author: Abotea
 *
 * Created on May 4, 2010, 2:33 PM
 */

#ifndef _RECTANGLESPLITTER_H
#define	_RECTANGLESPLITTER_H

#include "Rectangle.h"
#include "MoveTable.h"
#include "AbstractGraph.h"

class RectangleSplitter {
public:
    RectangleSplitter();
    RectangleSplitter(const RectangleSplitter& orig);
    virtual ~RectangleSplitter();
    void split(Rectangle & rec, const MoveTable & mt, const AbstractGraph * graph,
    vector<Rectangle> & result);
private:
    const static int TOP = 0;
    const static int RIGHT = 1;
    const static int BOTTOM = 2;
    const static int LEFT = 3;
    const static int MIDCOLUMN = 4;
    const static int MIDROW = 5;
    const static int NONE = 11;

    int area[4];
    int move[4];
    int noise[4];
    int line[4];
    int first_line_homog_idx[4];

    void computeLeftRect(const Rectangle & rec, const MoveTable & mt);
    void computeRightRect(const Rectangle & rec, const MoveTable & mt);
    void computeTopRect(const Rectangle & rec, const MoveTable & mt);
    void computeBottomRect(const Rectangle & rec, const MoveTable & mt);
    int computeWhereToSplit(const Rectangle & rec) const;
    void splitHorizontally(Rectangle & rec, const MoveTable & mt,
        int row, const AbstractGraph * graph,
        int homog_side, int areaval, int moveval, vector<Rectangle> & result);
    void splitVertically(Rectangle & rec, const MoveTable & mt,
        int col, const AbstractGraph * graph,
        int homog_side, int areaval, int moveval, vector<Rectangle> & result);
    void init(const Rectangle & rec, const MoveTable & mt, const AbstractGraph* graph);
    void computeLeftRect2(const Rectangle & rec, const MoveTable & mt, const AbstractGraph* graph);
    void computeRightRect2(const Rectangle & rec, const MoveTable & mt, const AbstractGraph* graph);
    void computeTopRect2(const Rectangle & rec, const MoveTable & mt, const AbstractGraph* graph);
    void computeBottomRect2(const Rectangle & rec, const MoveTable & mt, const AbstractGraph* graph);
    void resetAMNL(int idx) {
        assert (0 <= idx && idx < 4);
        area[idx] = 0;
        move[idx] = NODIRECTION;
        line[idx] = -1;
        noise[idx] = 10000;
        first_line_homog_idx[idx] = -1;
    }
};

#endif	/* _RECTANGLESPLITTER_H */

