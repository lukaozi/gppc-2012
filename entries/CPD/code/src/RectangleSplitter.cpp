/* 
 * File:   RectangleSplitter.cpp
 * Author: Abotea
 * 
 * Created on May 4, 2010, 2:33 PM
 */

#include "RectangleSplitter.h"
#include "utils.h"

RectangleSplitter::RectangleSplitter() {
}

RectangleSplitter::RectangleSplitter(const RectangleSplitter& orig) {
}

RectangleSplitter::~RectangleSplitter() {
}

void RectangleSplitter::computeLeftRect2(const Rectangle & rec,
        const MoveTable & mt, const AbstractGraph* graph) {
    // Set the column where to split and its noise level
    // Idea: grow to the right as long as the noise of the rectangle being built
    // is zero. If can't grow at least two columns wide, split in two at the half.
    int flags[MAX_NODE_DEGREE + 1];
    this->resetAMNL(LEFT);
    for (int idx = 0; idx <= MAX_NODE_DEGREE; idx++) {
        flags[idx] = 0;
    }
    for (int c = rec.getLeftCol(); c <= rec.getRightCol(); c++) {
        int area_increment = 0;
        const vector<int> & nodes = graph->getNodeIndexesOnColumn(c);
        for (unsigned int it = 0; it < nodes.size(); it++) {
            const AbstractGraphNode * pn = graph->getNode(nodes[it]);
            if (pn == NULL)
                continue;
            if (pn->getRow() > rec.getBottomRow())
                break;
            if (pn->getRow() < rec.getUpperRow())
                continue;
            //cerr << "node id: " << it << "; row: " << pn->getRow() << "; col: " << pn->getColumn() << endl;
            int idx = mt.getMove(nodes[it]);
            assert(idx >= 0);
            if (c == rec.getLeftCol() && idx != NODIRECTION &&
                    move[LEFT] != NODIRECTION && move[LEFT] != idx)
                first_line_homog_idx[LEFT] = pn->getRow() - 1;
            flags[idx] = 1;
            if (idx > 0) {
                if (move[LEFT] == NODIRECTION)
                    move[LEFT] = idx;
                area_increment++;
            }
        }
        int sum = 0;
        for (int idx = 1; idx <= MAX_NODE_DEGREE; idx++)
            sum += flags[idx];
        if (sum > 1) {
            break;
        }
        line[LEFT] = c;
        noise[LEFT] = 0;
        area[LEFT] += area_increment;
    }
    assert (line[LEFT] == -1 || noise[LEFT] == 0);
}

void RectangleSplitter::computeRightRect2(const Rectangle & rec,
        const MoveTable & mt, const AbstractGraph *graph) {
    int flags[MAX_NODE_DEGREE + 1];
    this->resetAMNL(RIGHT);
    for (int idx = 0; idx <= MAX_NODE_DEGREE; idx++) {
        flags[idx] = 0;
    }
    for (int c = rec.getRightCol(); c >= rec.getLeftCol(); c--) {
        int area_increment = 0;
        const vector<int> & nodes = graph->getNodeIndexesOnColumn(c);
        for (vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
            const AbstractGraphNode * pn = graph->getNode(*it);
            if (pn == NULL)
                continue;
            if (pn->getRow() > rec.getBottomRow())
                break;
            if (pn->getRow() < rec.getUpperRow())
                continue;
            //cerr << "node id: " << *it << "; row: " << pn->getRow() << "; col: " << pn->getColumn() << endl;
            int idx = mt.getMove(*it);
            assert(idx >= 0);
            flags[idx] = 1;
            if (c == rec.getLeftCol() && idx != NODIRECTION &&
                    move[RIGHT] != NODIRECTION && move[RIGHT] != idx)
                first_line_homog_idx[RIGHT] = pn->getRow() - 1;
            if (idx > 0) {
                if (move[RIGHT] == NODIRECTION)
                    move[RIGHT] = idx;
                area_increment++;
            }
        }
        int sum = 0;
        for (int idx = 1; idx <= MAX_NODE_DEGREE; idx++)
            sum += flags[idx];
        if (sum > 1) {
            break;
        }
        line[RIGHT] = c;
        noise[RIGHT] = 0;
        area[RIGHT] += area_increment;
    }
    assert (line[RIGHT] == -1 || noise[RIGHT] == 0);
}

void RectangleSplitter::computeTopRect2(const Rectangle & rec,
        const MoveTable & mt, const AbstractGraph* graph) {
    int flags[MAX_NODE_DEGREE + 1];
    this->resetAMNL(TOP);
    for (int idx = 0; idx <= MAX_NODE_DEGREE; idx++) {
        flags[idx] = 0;
    }
    for (int r = rec.getUpperRow(); r <= rec.getBottomRow(); r++) {
        int area_increment = 0;
        const vector<int> & nodes = graph->getNodeIndexesOnRow(r);
        for (int it = 0; it < nodes.size(); it++) {
            const AbstractGraphNode * pn = graph->getNode(nodes[it]);
            if (pn == NULL)
                continue;
            if (pn->getColumn() < rec.getLeftCol())
                continue;
            if (pn->getColumn() > rec.getRightCol())
                break;
            // cerr << "node id: " << *it << "; row: " << pn->getRow() << "; col: " << pn->getColumn() << endl;
            int idx = mt.getMove(nodes[it]);
            assert(idx >= 0);
            flags[idx] = 1;
            if (r == rec.getUpperRow() && idx != NODIRECTION &&
                    move[TOP] != NODIRECTION && move[TOP] != idx)
                first_line_homog_idx[TOP] = pn->getColumn() - 1;
            if (idx > 0) {
                if (move[TOP] == NODIRECTION)
                    move[TOP] = idx;
                area_increment++;
            }
        }
        int sum = 0;
        for (int idx = 1; idx <= MAX_NODE_DEGREE; idx++)
            sum += flags[idx];
        if (sum > 1) {
            break;
        }
        line[TOP] = r;
        noise[TOP] = 0;
        area[TOP] += area_increment;
    }
    assert (line[TOP] == -1 || noise[TOP] == 0);
}

void RectangleSplitter::computeBottomRect2(const Rectangle & rec,
        const MoveTable & mt, const AbstractGraph* graph) {
    int flags[MAX_NODE_DEGREE + 1];
    this->resetAMNL(BOTTOM);
    first_line_homog_idx[BOTTOM] = rec.getLeftCol();
    for (int idx = 0; idx <= MAX_NODE_DEGREE; idx++) {
        flags[idx] = 0;
    }
    for (int r = rec.getBottomRow(); r >= rec.getUpperRow(); r--) {
        int area_increment = 0;
        const vector<int> & nodes = graph->getNodeIndexesOnRow(r);
        for (vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
            const AbstractGraphNode * pn = graph->getNode(*it);
            if (pn == NULL)
                continue;
            if (pn->getColumn() > rec.getRightCol())
                break;
            if (pn->getColumn() < rec.getLeftCol())
                continue;
            // cerr << "node id: " << *it << "; row: " << pn->getRow() << "; col: " << pn->getColumn() << endl;
            int idx = mt.getMove(*it);
            assert(idx >= 0);
            flags[idx] = 1;
            if (r == rec.getUpperRow() &&  idx != NODIRECTION &&
                    move[BOTTOM] != NODIRECTION && move[BOTTOM] != idx)
                first_line_homog_idx[BOTTOM] = pn->getColumn() - 1;
            if (idx > 0) {
                if (move[BOTTOM] == NODIRECTION)
                    move[BOTTOM] = idx;
                area_increment++;
            }
        }
        int sum = 0;
        for (int idx = 1; idx <= MAX_NODE_DEGREE; idx++)
            sum += flags[idx];
        if (sum > 1) {
            break;
        }
        line[BOTTOM] = r;
        noise[BOTTOM] = 0;
        area[BOTTOM] += area_increment;
    }
    assert (line[BOTTOM] == -1 || noise[BOTTOM] == 0);
}

int RectangleSplitter::computeWhereToSplit(const Rectangle & rec) const {
    int max = -1, maxidx = -1;
    for (int x = TOP; x <= LEFT; x++) {
        if (noise[x] > 0) {
            assert (line[x] == -1);
            continue;
        }
        if (area[x] > max) {
            max = area[x];
            maxidx = x;
            assert (line[x] != -1);
            // cerr << x << " " << line[x] << rec  << endl;
        }
    }
    if (maxidx != -1)
        return maxidx;
    //cerr << "here" << endl;
    if (rec.getHeight() > rec.getWidth())
        return MIDROW;
    return MIDCOLUMN;
}

void RectangleSplitter::init(const Rectangle & rec,
        const MoveTable & mt, const AbstractGraph* graph) {
    this->computeLeftRect2(rec, mt, graph);
    if (line[LEFT] == rec.getRightCol())
        return; // this rectangle is homogeneous
    this->computeRightRect2(rec, mt, graph);
    this->computeTopRect2(rec, mt, graph);
    this->computeBottomRect2(rec, mt, graph);
}

void RectangleSplitter::split(Rectangle & rec, const MoveTable & mt,
        const AbstractGraph * graph,
        vector<Rectangle> & result) {
    //cerr << "splitting " << rec << endl;
    init(rec, mt, graph);
    if (line[LEFT] == rec.getRightCol()) {
        rec.setNrLocations(area[LEFT]);
        rec.setMove(move[LEFT]);
        // cerr << "rectangle area: " << area[LEFT] << endl;
        if (area[LEFT] > 0)
            result.push_back(rec);
        //cerr << "finished recursive splitting " << rec << endl;
        return;
    }
    assert (rec.getBottomRow() != rec.getUpperRow() ||
            rec.getLeftCol() != rec.getRightCol());
    // if we need to split
    int where_to_split = computeWhereToSplit(rec);
    int l;
    switch (where_to_split) {
        case LEFT:
            //cerr << "split left" << rec << endl;
            splitVertically(rec, mt, line[LEFT], graph, LEFT,
                    area[LEFT], move[LEFT], result);
            break;
        case RIGHT:
            //cerr << "split right" << rec << endl;
            splitVertically(rec, mt, line[RIGHT] - 1, graph, RIGHT,
                    area[RIGHT], move[RIGHT], result);
            break;
        case TOP:
            //cerr << "split top" << rec << endl;
            splitHorizontally(rec, mt, line[TOP], graph, TOP,
                    area[TOP], move[TOP], result);
            break;
        case BOTTOM:
            //cerr << "split bottom" << rec << endl;
            splitHorizontally(rec, mt, line[BOTTOM] - 1, graph, BOTTOM, 
                    area[BOTTOM], move[BOTTOM], result);
            break;
        case MIDCOLUMN:
            //cerr << "split mid column" << rec << endl;
            //l = first_line_homog_idx[BOTTOM];
            l = rec.getLeftCol() + (rec.getRightCol() - rec.getLeftCol()) / 2;
            splitVertically(rec, mt, l, graph, NONE, -1, -1, result);
            break;
        case MIDROW:
            //cerr << "split mid row" << rec << endl;
            //l = first_line_homog_idx[LEFT];
            l = rec.getUpperRow() + (rec.getBottomRow() - rec.getUpperRow()) / 2;
            splitHorizontally(rec, mt, l, graph, NONE, -1, -1, result);
            break;
        default:
            assert(false);
    }
}

void RectangleSplitter::splitVertically(Rectangle & rec,
        const MoveTable & mt, int col, const AbstractGraph * graph,
        int homog_side, int areaval, int moveval, vector<Rectangle> & result) {
    //homog_side = NONE;
    //cerr << "original rec: " << rec << " to split vertically at col " << col << endl;
    assert(rec.getLeftCol() <= col);
    Rectangle rec1(rec.getLeftCol(), rec.getUpperRow(), col, rec.getBottomRow());
    if (homog_side == LEFT) {
        rec1.setNrLocations(areaval);
        rec1.setMove(moveval);
        result.push_back(rec1);
    } else {
        split(rec1, mt, graph, result);
    }
    if (col < rec.getRightCol()) {
        Rectangle rec2(col + 1, rec.getUpperRow(), rec.getRightCol(), rec.getBottomRow());
        if (homog_side == RIGHT) {
            rec2.setNrLocations(areaval);
            rec2.setMove(moveval);
            result.push_back(rec2);
        } else {
            split(rec2, mt, graph, result);
        }
    }
}

void RectangleSplitter::splitHorizontally(Rectangle & rec,
        const MoveTable & mt, int row, const AbstractGraph * graph,
        int homog_side, int areaval, int moveval,
        vector<Rectangle> & result) {
    //homog_side = NONE;
    //cerr << "original rec: " << rec << " to split horizontally at row " << row << endl;
    Rectangle rec1(rec.getLeftCol(), rec.getUpperRow(), rec.getRightCol(), row);
    if (homog_side == TOP) {
        rec1.setNrLocations(areaval);
        rec1.setMove(moveval);
        result.push_back(rec1);
    } else {
        split(rec1, mt, graph, result);
    }
    if (row < rec.getBottomRow()) {
        Rectangle rec2(rec.getLeftCol(), row + 1, rec.getRightCol(), rec.getBottomRow());
        if (homog_side == BOTTOM) {
            rec2.setNrLocations(areaval);
            rec2.setMove(moveval);
            result.push_back(rec2);
        } else {
            split(rec2, mt, graph, result);
        }
    }
}
