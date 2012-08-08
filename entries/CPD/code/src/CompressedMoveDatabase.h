/* 
 * File:   CompressedMoveDatabase.h
 * Author: Abotea
 *
 * Created on April 13, 2010, 9:54 AM
 */

#ifndef _COMPRESSEDMOVEDATABASE_H
#define	_COMPRESSEDMOVEDATABASE_H

#include <vector>
#include "Rectangle.h"
#include "globals.h"
#include "CompressedTable.h"
#include "GridMap.h"
#include "AbstractGraph.h"
#include "GenericMap.h"
#include <stdlib.h>
#include <string>
#include "AbstractCPD.h"


using namespace std;

class CompressedMoveDatabase : public AbstractCPD {
public:
    CompressedMoveDatabase();

    CompressedMoveDatabase(AbstractGraph *pg)
    : pg(pg), first_node_idx(0), last_node_idx(pg->getNrNodes() - 1),
    nr_rectangles_before_demo(0), nr_1x1_rectangles(0)
    {
    };

    CompressedMoveDatabase(AbstractGraph *pg, int first, int last)
    : pg(pg), first_node_idx(first), last_node_idx(last),
        nr_rectangles_before_demo(0), nr_1x1_rectangles(0)
    {
    };
    CompressedMoveDatabase(const CompressedMoveDatabase& orig);
    virtual ~CompressedMoveDatabase();
    void build(const GridMap & map);
    void build(bool list_trimming);

    void addCompressedTable(int idx, const CompressedTable & table) {
        //assert(idx == tables2.size());
        this->tables2[idx] = table;
    }

    double getAverageChecks() const {
        return average_checks;
    }

    const vector<double> & getCoveredAreaStats() const {
        return this->covered_area_stats;
    }

    int getMove(int startIdx, int targetIdx, int & nr_rect_checks,
            const AbstractGraph * graph) {
        int target_col = (graph->getNode(targetIdx))->getColumn();
        int target_row = graph->getNode(targetIdx)->getRow();
        int orig_col = graph->getNode(startIdx)->getColumn();
        int orig_row = graph->getNode(startIdx)->getRow();
        return tables2[startIdx].getMove(Tile(target_col, target_row, -1),
            orig_col, orig_row, nr_rect_checks, graph);
    }
    
    int getNrRectangles() const {
        return nr_rectangles;
    }
    void readDataBinary(ifstream & infile);
    void writeDataBinary(ofstream & outfile) const;
    void writeDataBinaryNoHeader(ofstream & outfile) const;    
    void readDataFromStream(ifstream & infile);
    void readDataFromStream2(ifstream & infile);
    void writeDataToStream(ofstream & outfile, const GridMap & map) const;
    void writeDataToStream(ofstream & outfile) const;
    int getNrRectanglesBeforeDeMoFiltering() const {
        return nr_rectangles_before_demo;
    }
    int getNr1x1Rectangles() const {
        return nr_1x1_rectangles;
    }
private:
    AbstractGraph *pg;
    int first_node_idx;
    int last_node_idx;
    vector<CompressedTable> tables2;
    // some stats:
    // How many rectangles there are in total
    int nr_rectangles;
    // Average number of checks to find the right rectangle in a list
    double average_checks;
    // covered_area_stats[i] is the fraction of the map
    // covered by the first i rectangles,
    // averaged over all compressed tables
    vector<double> covered_area_stats;
    int nr_rectangles_before_demo;
    int nr_1x1_rectangles;
    void computeCoveredAreaStats(const GridMap & map);
    void computeCoveredAreaStats(const AbstractGraph * graph);
};

#endif	/* _COMPRESSEDMOVEDATABASE_H */
