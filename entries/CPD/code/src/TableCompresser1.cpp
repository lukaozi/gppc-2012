/* 
 * File:   TableCompresser1.cpp
 * Author: abotea
 * 
 * Created on 27 April 2011, 5:33 PM
 */

#include "TableCompresser1.h"
#include "RectangleSplitter.h"
#include "utils.h"

TableCompresser1::TableCompresser1() {
    counter = 0;
    this->nr_1x1_rectangles = 0;
    this->nr_rectangles_before_demo = 0;
}

TableCompresser1::TableCompresser1(const TableCompresser1& orig) {
}

TableCompresser1::~TableCompresser1() {
}

CompressedTable TableCompresser1::compressTable(int col, int row,
        const MoveTable & mt, const AbstractGraph * graph, bool list_trimming) {
    return compressTable2(col, row, mt, graph, list_trimming);
    /*
    CompressedTable result1 = compressTable1(col, row, mt, graph);
    CompressedTable result2 = compressTable2(col, row, mt, graph);
    if (result1.getNrRectangles() < result2.getNrRectangles())
        return result1;
    return result2;
     * */
}

CompressedTable TableCompresser1::compressTable9(int col, int row,
        const MoveTable & mt, const AbstractGraph * graph, bool list_trimming) {
    CompressedTable result;
    Rectangle rectangleNW(0, 0, col-1, row-1); // upper left
    Rectangle rectangleNE(col+1, 0, graph->getWidth()-1, row-1); // upper right
    Rectangle rectangleSE(col+1, row+1, graph->getWidth()-1, graph->getHeight()-1); // lower right
    Rectangle rectangleSW(0, row+1, col-1, graph->getHeight()-1); // lower left
    Rectangle rectangleN(col, 0, col, row-1); // NORTH
    Rectangle rectangleE(col+1, row, graph->getWidth()-1, row); // east
    Rectangle rectangleS(col, row+1, col, graph->getHeight()-1); // south
    Rectangle rectangleW(0, row, col-1, row); // west

    RectangleSplitter splitter;
    vector<Rectangle> rectangles0, rectangles1, rectangles2, rectangles3;
    vector<Rectangle> fr0, fr1, fr2, fr3;
    int default_move;
    this->nr_rectangles_before_demo = 0;
    this->nr_1x1_rectangles = 0;
    splitter.split(rectangleNW, mt, graph, rectangles0);
    processRectangles(rectangles0, fr0, default_move, mt, graph, list_trimming);
    result.addRectangles(fr0, 0);
    result.setDefaultMove(default_move, 0);
    splitter.split(rectangleNE, mt, graph, rectangles1);
    processRectangles(rectangles1, fr1, default_move, mt, graph, list_trimming);
    result.addRectangles(fr1, 1);
    result.setDefaultMove(default_move, 1);
    splitter.split(rectangleSE, mt, graph, rectangles2);
    processRectangles(rectangles2, fr2, default_move, mt, graph, list_trimming);
    result.addRectangles(fr2, 2);
    result.setDefaultMove(default_move, 2);
    splitter.split(rectangleSW, mt, graph, rectangles3);
    processRectangles(rectangles3, fr3, default_move, mt, graph, list_trimming);
    result.addRectangles(fr3, 3);
    result.setDefaultMove(default_move, 3);
    return result;
}

CompressedTable TableCompresser1::compressTable2(int col, int row,
        const MoveTable & mt, const AbstractGraph * graph, bool list_trimming) {
    CompressedTable result;
    Rectangle rectangle0(0, 0, col, row); // upper left
    Rectangle rectangle1(col+1, 0, graph->getWidth()-1, row); // upper right
    Rectangle rectangle2(col+1, row+1, graph->getWidth()-1, graph->getHeight()-1); // lower right
    Rectangle rectangle3(0, row+1, col, graph->getHeight()-1); // lower left
    RectangleSplitter splitter;
    vector<Rectangle> rectangles0, rectangles1, rectangles2, rectangles3;
    vector<Rectangle> fr0, fr1, fr2, fr3;
    int default_move;
    this->nr_rectangles_before_demo = 0;
    this->nr_1x1_rectangles = 0;
    splitter.split(rectangle0, mt, graph, rectangles0);
    processRectangles(rectangles0, fr0, default_move, mt, graph, list_trimming);
    result.addRectangles(fr0, 0);
    result.setDefaultMove(default_move, 0);
    splitter.split(rectangle1, mt, graph, rectangles1);
    processRectangles(rectangles1, fr1, default_move, mt, graph, list_trimming);
    result.addRectangles(fr1, 1);
    result.setDefaultMove(default_move, 1);
    splitter.split(rectangle2, mt, graph, rectangles2);
    processRectangles(rectangles2, fr2, default_move, mt, graph, list_trimming);
    result.addRectangles(fr2, 2);
    result.setDefaultMove(default_move, 2);
    splitter.split(rectangle3, mt, graph, rectangles3);
    processRectangles(rectangles3, fr3, default_move, mt, graph, list_trimming);
    result.addRectangles(fr3, 3);
    result.setDefaultMove(default_move, 3);
    return result;
}

void TableCompresser1::processRectangles(vector<Rectangle> & rectangles, 
        vector<Rectangle> & fr,
        int & default_move,
        const MoveTable & mt, const AbstractGraph * graph,
        bool list_trimming) {
    this->nr_rectangles_before_demo += rectangles.size();
    multiset<Rectangle> rectangles2;
    vector<Rectangle>::iterator it;
    // order list of rectangles
    for (it = rectangles.begin(); it != rectangles.end(); it++) {
        if (it->getMove() == NODIRECTION)
            continue;
        rectangles2.insert(*it);
    }
    vector<Rectangle> filtered_rectangles;
    default_move = 0;
    if (list_trimming) {
        filter_rectangles_by_default_move2(rectangles2, 
                filtered_rectangles, default_move);
    } else {
        multiset<Rectangle>::const_iterator lit;
        for (lit = rectangles2.begin(); lit != rectangles2.end(); lit++) {
            filtered_rectangles.push_back(*lit);
        }
    }
    assert (fr.size() == 0);
    for (it = filtered_rectangles.begin(); it != filtered_rectangles.end(); it++) {
        // deactivate rectangle shrinking because the current implementation
        // doesn't work correctly for an allocatin of 11 bits per row/col value
        if (false && it->getNrLocations() == 1) {
            nr_1x1_rectangles++;
            assert (nr_1x1_rectangles <= nr_rectangles_before_demo);
            Rectangle r = shrinkTo1x1(*it, graph, mt);
            fr.push_back(r);
        } else
            fr.push_back(*it);
    }
    assert (fr.size() <= filtered_rectangles.size());
}

vector<Rectangle> TableCompresser1::filter_rectangles_by_default_move1(vector<Rectangle> & rectangles,
        const MoveTable & mt, const AbstractGraph * graph) const {
    vector<Rectangle> filtered_rectangles;
    for (int i = 0; i < rectangles.size(); i++) {
        if (!rectangle_encodes_default_moves(mt, graph, rectangles[i]))
            filtered_rectangles.push_back(rectangles[i]);
    }
    return filtered_rectangles;
}

void TableCompresser1::filter_rectangles_by_default_move2(multiset<Rectangle> & rectangles,
         vector<Rectangle> & filtered_rectangles, int & default_move) const {
    int counters[MAX_NODE_DEGREE + 1];
    for (int i = 0; i < MAX_NODE_DEGREE + 1; i++)
        counters[i] = 0;
    multiset<Rectangle>::const_iterator it;
    int i = -1;
    for (it = rectangles.begin(); it != rectangles.end(); it++) {
        i++;
        if (skip_rectangle(i, rectangles.size(), it->getNrLocations()))
            continue;
        counters[it->getMove()]++;
    }
    int max = 0;
    int maxidx = -1;
    for (int i = 1; i < MAX_NODE_DEGREE + 1; i++) {
        if (counters[i] > max) {
            max = counters[i];
            maxidx = i;
        }
    }
    default_move = maxidx;
    assert (default_move != 0);
    i = 0;
    for (it = rectangles.begin(); it != rectangles.end(); it++) {
        if (skip_rectangle(i, rectangles.size(), it->getNrLocations()))
            filtered_rectangles.push_back(*it);
        else if (it->getMove() != default_move)
            filtered_rectangles.push_back(*it);
        i++;
    }
}

void TableCompresser1::filter_rectangles_by_default_move3(vector<Rectangle> & rectangles,
        const MoveTable & mt, const AbstractGraph * graph, 
        vector<Rectangle> & filtered_rectangles, int * default_moves) const {
    int tmp[4][MAX_NODE_DEGREE + 1];
    int dominant_moves[4];
    for (int quarter = 0; quarter < 4; quarter++) {
        dominant_moves[quarter] = -1;
        for (int j = 0; j <= MAX_NODE_DEGREE; j++)
            tmp[quarter][j] = 0;
    }

    for (int i = 0; i < rectangles.size(); i++) {
        const Rectangle & rect = rectangles[i];
        if (skip_rectangle(i, rectangles.size(), rectangles[i].getNrLocations()))
            continue;
        int quarter = detect_quarter(rect, mt);
        tmp[quarter][rect.getMove()]++;
    }
    for (int quarter = 0; quarter < 4; quarter++) {
        int max = 0;
        for (int j = 1; j <= MAX_NODE_DEGREE; j++) {
            if (tmp[quarter][j] > max) {
                max = tmp[quarter][j];
                dominant_moves[quarter] = j;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        default_moves[i] = dominant_moves[i];
        assert (default_moves[i] != 0);
        //cerr << "default move for quarter" << i << " is " << default_moves[i] << endl;
    }
    for (int i = 0; i < rectangles.size(); i++) {
        const Rectangle & rect = rectangles[i];
        //cerr << "looking at rect " << rect << endl;
        if (skip_rectangle(i, rectangles.size(), rect.getNrLocations())) {
            filtered_rectangles.push_back(rect);
            //cerr << "keeping rect " << rect << endl;
        } else {
            int quarter = detect_quarter(rect, mt);
            if (default_moves[quarter] != rect.getMove()) {
                filtered_rectangles.push_back(rect);
                //cerr << "keeping rect " << rect << endl;
            }
        }
    }
}

bool TableCompresser1::skip_rectangle(int position_in_list, int list_size, int nr_locations) const {
    if (position_in_list < list_size/10 && nr_locations > 1)
        return true;
    return false;
}

bool TableCompresser1::rectangle_encodes_default_moves(const MoveTable & mt,
        const AbstractGraph * graph, const Rectangle & rec) const {
    int label = rec.getMove();
    AbstractGraphNode *po = graph->getNode(mt.getOriginIdx());
    for (int c = rec.getLeftCol(); c <= rec.getRightCol(); c++) {
        const vector<int> & nodes = graph->getNodeIndexesOnColumn(c);
        for (unsigned int it = 0; it < nodes.size(); it++) {
            const AbstractGraphNode * pn = graph->getNode(nodes[it]);
            if (pn == NULL)
                continue;
            if (pn->getRow() > rec.getBottomRow())
                continue;
            if (pn->getRow() < rec.getUpperRow())
                continue;
            //cerr << "node id: " << it << "; row: " << pn->getRow() << "; col: " << pn->getColumn() << endl;
            if (pn == po)
                continue;
            if (mt.getMove(nodes[it]) == 0) {
                //cerr << "here" << endl;
                continue;
            }
            int default_move = graph->calculateDefaultMove(po, pn);
            if (default_move != label)
                return false;
        }
    }
    return true;
}

CompressedTable TableCompresser1::compressTable1(int col, int row,
        const MoveTable & mt, const AbstractGraph * graph) {
    CompressedTable result;
    // big rectangle that covers entire map
    Rectangle rectangle(0, 0, graph->getWidth() - 1, graph->getHeight() - 1);
    RectangleSplitter splitter;
    vector<Rectangle> rectangles;
    splitter.split(rectangle, mt, graph, rectangles);
    result.addRectangles(rectangles, 0); // 0 is a stupid value to make it compile
    assert (false);
    return result;
}

int TableCompresser1::mergeRectsInList(vector<Rectangle> & rectangles) {
    for (int i = 0; i < rectangles.size(); i++) {
        Rectangle cr1 = rectangles[i];
        for (int j = i+1; j < rectangles.size(); j++) {
            Rectangle cr2 = rectangles[j];
            if (cr1.getMove() == cr2.getMove()) {
                if (cr1.getHeight() == cr2.getHeight() &&
                    cr1.getLeftCol() == cr2.getRightCol() + 1)
                    counter++;
                if (cr1.getHeight() == cr2.getHeight() &&
                    cr2.getLeftCol() == cr1.getRightCol() + 1)
                    counter++;
                if (cr1.getWidth() == cr2.getWidth() &&
                    cr2.getUpperRow() == cr1.getBottomRow() + 1)
                    counter++;
                if (cr1.getWidth() == cr2.getWidth() &&
                    cr1.getUpperRow() == cr2.getBottomRow() + 1)
                    counter++;
            }
        }
    }
    return counter;
}

int TableCompresser1::detect_quarter(const Rectangle &r, const MoveTable &mt) const {
    int quarter = -1;
    // detect quarter
    int rrc = max(r.getLeftCol(), r.getRightCol());
    int rbr = max(r.getUpperRow(), r.getBottomRow());
    if (rrc <= mt.getOrigCol() && rbr <= mt.getOrigRow()) {
        quarter = 0;
    } else if (r.getLeftCol() > mt.getOrigCol() && rbr <= mt.getOrigRow()) {
        quarter = 1;
    } else if (r.getLeftCol() > mt.getOrigCol() && r.getUpperRow() > mt.getOrigRow()) {
        quarter = 2;
    } else if (rrc <= mt.getOrigCol() && r.getUpperRow() > mt.getOrigRow()) {
        quarter = 3;
    }
    assert (quarter >= 0);
    return quarter;
}

Rectangle TableCompresser1::shrinkTo1x1(const Rectangle & r, 
        const AbstractGraph * graph, const MoveTable & mt) const {
    assert (r.getNrLocations() == 1);
    for (int c = r.getLeftCol(); c <= r.getRightCol(); c++) {
        const vector<int> & nodes = graph->getNodeIndexesOnColumn(c);
        for (unsigned int it = 0; it < nodes.size(); it++) {
            const AbstractGraphNode * pn = graph->getNode(nodes[it]);
            if (pn == NULL)
                continue;
            if (pn->getRow() > r.getBottomRow())
                break;
            if (pn->getRow() < r.getUpperRow())
                continue;
            if (mt.getMove(pn->getGlobalIdx()) == 0)
                continue; // this is an unreachable node
            // now we reached the only location contained in this rectangle
            //Rectangle result(pn->getColumn(), pn->getRow(), 0, 0, r.getNrLocations(), r.getMove());
            Rectangle result(pn->getGlobalIdx(), 0, 0, 0, r.getNrLocations(), r.getMove());
            return result;
        }
    }
}
