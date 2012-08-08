/* 
 * File:   Scenario.h
 * Author: Abotea
 *
 * Created on May 31, 2010, 5:14 PM
 */

#ifndef _SCENARIO_H
#define	_SCENARIO_H

#include <stdlib.h>
#include <iostream>
#include <assert.h>


using namespace std;

struct Record {
    std::string mapfile;
    int start_col;
    int start_row;
    int target_col;
    int target_row;
    double cost;
};

class Scenario {
public:
    Scenario() :nr_records(0) {};
    Scenario(const Scenario& orig);
    virtual ~Scenario();
    int getNrRecords() const {
        return nr_records;
    }
    const Record & getRecord(int index) {
        assert (index >= 0 && index < nr_records);
        return records[index];
    }
    void readFromFile(const char* filename);
private:
    Record records[5000];
    int nr_records;
};

#endif	/* _SCENARIO_H */

