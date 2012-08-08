/* 
 * File:   MapCpd.h
 * Author: adi
 *
 * Created on 23 April 2012, 10:05 PM
 */

#ifndef MAPCPD_H
#define	MAPCPD_H

#include <vector>
#include "GridMap.h"
#include "CompressedMoveDatabase.h"

class MapCpd {
public:
    MapCpd();
    MapCpd(std::vector<bool> &bits, int w, int h, const char *filename);
    MapCpd(const MapCpd& orig);
    virtual ~MapCpd();
    GridMap * getGraph() const {
        return pg;
    };
    CompressedMoveDatabase * getCPD() const {
        return pc;
    };
private:
    GridMap * pg;
    CompressedMoveDatabase * pc;
};

#endif	/* MAPCPD_H */

