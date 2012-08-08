/* 
 * File:   Scenario.cpp
 * Author: Abotea
 * 
 * Created on May 31, 2010, 5:14 PM
 */

#include "Scenario.h"
#include <fstream>

using namespace std;


Scenario::Scenario(const Scenario& orig) {
}

Scenario::~Scenario() {
}

void Scenario::readFromFile(const char* filename) {
    ifstream infile;
    cerr << "Opening input file " << filename << endl;
    infile.open(filename);
    if (!infile) {
        cerr << "Unable to open " << filename << endl;
        exit (1);
    }
    string token;
    infile >> token;
    assert (token == "version");
    infile >> token;
    assert (token == "1.0");
    while (!infile.eof()) {
        infile >> token; // id of the record
        infile >> records[nr_records].mapfile;
        infile >> token; // one dimension of the map
        infile >> token; // the other dimension
        infile >> records[nr_records].start_col;
        infile >> records[nr_records].start_row;
        infile >> records[nr_records].target_col;
        infile >> records[nr_records].target_row;
        infile >> records[nr_records].cost;
        nr_records++;
    }
    nr_records--;
}
