/* 
 * File:   ScenarioRM.cpp
 * Author: abotea
 * 
 * Created on 17 December 2010, 1:44 PM
 */

#include "ScenarioRM.h"

ScenarioRM::ScenarioRM(const ScenarioRM& orig) {
}

ScenarioRM::~ScenarioRM() {
}

void ScenarioRM::readFromFile(const string & filename) {
    ifstream infile;
    cerr << "Opening input file " << filename << endl;
    infile.open(filename.c_str());
    if (!infile) {
        cerr << "Unable to open " << filename << endl;
        exit (1);
    }
    int some_number;
    infile >> some_number;
    while (!infile.eof()) {
        RecordRM record;
        infile >> record.start_idx;
        infile >> record.target_idx;
        record.cost = -1.0;
        this->records.push_back(record);
    }
}
