

#include "utils.h"
#include "globals.h"
#include <iomanip>

bool comp(AbstractGraphNode* n1, AbstractGraphNode* n2) {
    return (*n1) < (*n2);
}

ostream & operator << (ostream &os, const CompressedMoveDatabase & cmd) {
    const vector<double> & stats = cmd.getCoveredAreaStats();
    //os << " -- Nr rectangles: " << cmd.getNrRectangles() << endl;
    os << " -- Average nr rectangle checks: " << cmd.getAverageChecks() << endl;
    os << " -- Max nr rectangles per list: " << cmd.getCoveredAreaStats().size() << endl;
    os << " -- Area coverage stats in 3-digit precision: ";
    for (unsigned int i = 0; i < stats.size(); i++) {
        os << i + 1 << " " << setprecision(3) << stats[i];
        if (i > 30) {
            cerr << "...";
            break;
        }
        else cerr << "; ";
    }
    os << endl;
    return os;
}

ostream & operator << (ostream &os, const CompressedTable & ct) {
    int count = 0;
    //const SlimmerRectangle* elems = ct.getRectangles();
    for (int q = 0; q < 4; q++) {
        os << "List of rectangles for quarter " << q << ":" << endl;
        for (int it = 0; it != ct.getNrRectangles(q); it++) {
            count++;
            os << "Rectangle: ";
            os << ct.getLeftCol(it, q) << ", ";
            os << ct.getUpperRow(it, q) << ", ";
            os << ct.getRightCol(it, q) << ", ";
            os << ct.getBottomRow(it, q) << ", ";
            os << ct.getMove(it, q) << endl << endl;
        }
    }
    os << "Total nr rectangles: " << count << endl;
    return os;
}

ostream & operator << (ostream &os, const Rectangle & rectangle) {
    os << "[" << rectangle.getLeftCol() << "," << rectangle.getUpperRow();
    os << ";" << rectangle.getRightCol() << "," << rectangle.getBottomRow();
    os << "] nr locations: " << rectangle.getNrLocations();
    os << "; move: " << rectangle.getMove();
    return os;
}

ostream & operator << (ostream &os, const SlimRectangle & rectangle) {
    os << "[" << rectangle.getLeftCol() << "," << rectangle.getUpperRow();
    os << ";" << rectangle.getRightCol() << "," << rectangle.getBottomRow();
    os << "]; move: " << rectangle.getMove();
    return os;
}

ostream & operator << (ostream &os, const SlimmerRectangle & rectangle) {
    os << "[" << rectangle.getLeftCol() << "," << rectangle.getUpperRow();
    os << ";" << rectangle.getRightCol() << "," << rectangle.getBottomRow();
    os << "]; move: " << rectangle.getMove();
    return os;
}

ostream & operator << (ostream &os, const GenericNode & node) {
	os << "[" << node.getGlobalIdx();
        os << "; " << node.getStringId() << "]" << endl;
	vector<AbstractGraphEdge*>::const_iterator it;
	for (it = node.getEdges().begin(); it != node.getEdges().end(); it++) {
		os << "    " << *it << endl;
	}
	return os;
}

ostream & operator << (ostream &os, const GenericEdge & edge) {
	os << "[" << edge.getId() << " : ";
	os << edge.getNode1Idx() << " --> " << edge.getNode2Idx() << "]";
	return os;
}
