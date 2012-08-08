/* 
 * File:   ScenarioRM.h
 * Author: abotea
 *
 * Created on 17 December 2010, 1:44 PM
 */

#ifndef _SCENARIORM_H
#define	_SCENARIORM_H

#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct RecordRM {
    int start_idx;
    int target_idx;
    double cost;
};

class ScenarioRM {
public:
    ScenarioRM()
    {};
    ScenarioRM(const ScenarioRM& orig);
    virtual ~ScenarioRM();
    int getNrRecords() const {
        return records.size();
    }
    const RecordRM & getRecord(int index) {
        assert (index >= 0 && index < records.size());
        return records[index];
    }
    void readFromFile(const string & filename);
private:
    vector<RecordRM> records;
};


#endif	/* _SCENARIORM_H */

