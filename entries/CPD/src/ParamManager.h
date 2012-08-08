/* 
 * File:   ParamManager.h
 * Author: Abotea
 *
 * Created on May 31, 2010, 4:37 PM
 */

#ifndef _PARAMMANAGER_H
#define	_PARAMMANAGER_H

#include <assert.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "GridMap.h"
#include "GenericMap.h"
#include "MoveTable.h"
#include "CompressedMoveDatabase.h"
#include "utils.h"
#include "ScenarioRM.h"
#include <time.h>
#include "ReachabilityTable.h"
#include "QTDatabase.h"

using namespace std;

class ParamManager {
public:
    ParamManager();
    ParamManager(int argc, char ** argv) {
        this->nr_args = argc;
        for (int i = 0; i < nr_args; i++)
            this->args.push_back(argv[i]);
        this->analyze_params();
    }
    ParamManager(const ParamManager& orig);
    virtual ~ParamManager();
    void analyze_params();
    bool is_mode_preprocessing_gridmap() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 1);
    }
    bool is_mode_printing() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 3);
    }
    bool is_mode_pathfinding() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 2);
    }
    bool is_mode_pathfinding_rm() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 5);
    }
    bool is_mode_preprocessing_roadmap() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 4);
    }
    bool is_mode_preproc_comp() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 10);
    }
    bool is_mode_pf_comp() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 11);
    }
    bool is_mode_build_cpd_rd() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return (mode == 9);
    }
    bool is_mode_build_qt_roadmap() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return mode == 6;
    }
    bool is_mode_build_qt_gridmap() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return mode == 7;
    }
    bool is_mode_pf_cpd2() const {
        assert (mode >= 1 && mode <= MAX_MODES);
        return mode == 8;
        
    }
    void call_appropriate_method() const;
private:
    void display_usage_info() const;
    void build_qt_gm() const;
    void preprocess_rm(bool list_trimming) const;
    void preprocess_gm() const;
    void preprocess_comp() const;
    void find_paths_comp() const;
    void build_qt_rm() const;
    void find_paths_rm() const;
    void find_paths_gm() const;
    void print_preprocessing_stats(const CompressedMoveDatabase & cmd,
        double t, const string & filename, int nr_nodes) const;
    void print_build_qt_stats(const AbstractGraph* graph,
        const QTDatabase & qtd, double t, const string & filename, int nr_nodes) const;
    
    // running mode: 1 for building cpd, 2 for doing pathfinding
    int mode;
    int nr_args;
    vector<string> args;
    static const int MAX_MODES = 11;
};

#endif	/* _PARAMMANAGER_H */

