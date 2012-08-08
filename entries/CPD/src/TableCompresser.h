/* 
 * File:   TableCompresser.h
 * Author: abotea
 *
 * Created on 27 April 2011, 5:30 PM
 */

#ifndef _TABLECOMPRESSER_H
#define	_TABLECOMPRESSER_H

#include "CompressedTable.h"
#include "MoveTable.h"
#include "AbstractGraph.h"

class TableCompresser {
public:
    TableCompresser();
    TableCompresser(const TableCompresser& orig);
    virtual ~TableCompresser();
    virtual CompressedTable compressTable(int col, int row,
            const MoveTable & mt, const AbstractGraph * graph, bool list_trimming) = 0;
    virtual int getNrRectanglesBeforeDeMoFiltering() const = 0;
    virtual int getNr1x1Rectangles() const = 0;
private:

};

#endif	/* _TABLECOMPRESSER_H */

