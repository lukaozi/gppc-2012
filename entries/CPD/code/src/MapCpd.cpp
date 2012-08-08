/* 
 * File:   MapCpd.cpp
 * Author: adi
 * 
 * Created on 23 April 2012, 10:05 PM
 */

#include "MapCpd.h"
#include <assert.h>

MapCpd::MapCpd() {
}

MapCpd::MapCpd(std::vector<bool> &bits, int w, int h, const char *filename) {
    ifstream in_cpd_file;
    //assert (pg == NULL);
    pg = new GridMap(bits, w, h);
    //assert (pc == NULL);
    pc = new CompressedMoveDatabase();
    in_cpd_file.open(filename, ios::binary); // cpd file
    if (!in_cpd_file) {
        cerr << "Unable to open " << filename << endl;
        delete pg;
        delete pc;
        exit (1);
    }
    pc->readDataBinary(in_cpd_file);
    in_cpd_file.close();
}

MapCpd::MapCpd(const MapCpd& orig) {
}

MapCpd::~MapCpd() {
    if (pg != NULL) {
        delete pg;
        pg = NULL;
    }
    if (pc != NULL) {
        delete pc;
        pc = NULL;
    }
}
