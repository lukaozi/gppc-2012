/* 
 * File:   ParamManager.cpp
 * Author: Abotea
 * 
 * Created on May 31, 2010, 4:37 PM
 */

#include <iomanip>
#include "ParamManager.h"
#include "AbstractCPD.h"
#include "CPD2.h"
#include "Entry.h"
#include "MapCpd.h"
#include "Scenario.h"

using namespace std;

ParamManager::ParamManager() {
}

ParamManager::ParamManager(const ParamManager& orig) {
}

ParamManager::~ParamManager() {
}

void ParamManager::analyze_params() {
    if (nr_args == 1) {
        display_usage_info();
        exit(1);
    }
    if (args[1] == "build_cpd_gm") {
        cerr << "OBSOLETE OPTION" << endl;
        //if (nr_args != 5) {
        display_usage_info();
        exit(1);
        mode = 1;
    } else if (args[1] == "pathfinding_gm") {
        cerr << "OBSOLETE OPTION" << endl;
        //if (nr_args != 5) {
        display_usage_info();
        exit(1);
        mode = 2;
    } else if (args[1] == "printing") {
        cerr << "OBSOLETE OPTION" << endl;
        //if (nr_args != 6) {
        display_usage_info();
        exit(1);
        mode = 3;
    } else if (args[1] == "build_cpd_rm" || args[1] == "build_cpd_rd_lt") {
        if (nr_args != 6 && nr_args != 8) {
            display_usage_info();
            exit(1);
        }
        mode = 4;
    } else if (args[1] == "build_cpd_rd") {
        if (nr_args != 6 && nr_args != 8) {
            display_usage_info();
            exit(1);
        }
        mode = 9;
    } else if (args[1] == "preprocess_competition") {
        if (nr_args != 4) {
            display_usage_info();
            exit(1);
        }
        mode = 10;
    } else if (args[1] == "pathfinding_rm") {
        if (nr_args < 8) {
            display_usage_info();
            exit(1);
        }
        mode = 5;
    } else if (args[1] == "pathfinding_competition") {
        if (nr_args != 6) {
            display_usage_info();
            exit(1);
        }
        mode = 11;
    } else if (args[1] == "build_qt_rm") {
        if (nr_args < 4) {
            display_usage_info();
            exit(1);
        }
        mode = 6;
    } else if (args[1] == "build_qt_gm") {
        cerr << "OBSOLETE OPTION" << endl;
        //if (nr_args < 2) {
        display_usage_info();
        exit(1);
        mode = 7;
    } else if (args[1] == "pathfinding_cpd2") {
        if (nr_args < 2) {
            display_usage_info();
            exit(1);
        }
        mode = 8;
    } else {
        display_usage_info();
        exit(1);
    }
}

void ParamManager::display_usage_info() const {
    cerr << "Usage: ./cpd mode" << endl;
    cerr << "    Mode1 OBSOLETE: build_cpd_gm map_file cpd_file rt_file" << endl;
    cerr << "    Mode2 OBSOLETE: pathfinding_gm cpd_file scenario_file results_file" << endl;
    cerr << "    Mode OBSOLETE 3: printing map_file print_type column row" << endl;
    cerr << "      print_type values: 0 print distance table;" << endl;
    cerr << "                         1 print first-move table" << endl;
    cerr << "                         2 print empty grid" << endl;
    cerr << "    Mode4: build_cpd_rm vertex_file edge_file cpd_file ";
    cerr << "reach_table_file [first_node_idx last_node_idx]" << endl;
    cerr << "        Use NO_REACH_TABLE for reach_table_file to skip ";
    cerr << "reachability table computation" << endl;
    cerr << "        The last two, optional parameters allow to compute";
    cerr << " only a part of a CPD" << endl;
    cerr << "    Mode9: build_cpd_rd [same remaining args as in mode 4]" << endl;
    cerr << "           This builds a CPD only with rectangle decomposition," << endl;
    cerr << "           skipping subsequent steps such as list trimming" << endl;
    cerr << "    Mode5: pathfinding_rm vertex_file edge_file ";
    cerr << "cpd_file reach_table_file scenario_file output_file" << endl;
    cerr << "    Mode6: build_qt_rm vertex_file edge_file [first_node_idx last_node_idx]" << endl;
    cerr << "    Mode7 OBSOLETE: build_qt_gm map_file [first_node_idx last_node_idx]" << endl;
    cerr << "    Mode8: pathfinding_cpd2 vertex_file edge_file ";
    cerr << "cpd2_file reach_table_file scenario_file output_file" << endl;
    cerr << "    Mode10: preprocess_competition map_file cpd_file" << endl;
    cerr << "    Mode11: pathfinding_competition map_file cpd_file scenario_file output_file" << endl;
    cerr <<  "           Preprocessing must be done in mode 10" << endl;
}

void ParamManager::preprocess_rm(bool list_trimming) const {
    cerr << "files: " << args[2] << "; " << args[3] << "; ";
    cerr << args[4] << "; " << args[5] << endl;
    time_t start, end;
    time(&start);
    AbstractGraph * pg = new GenericMap(args[2], args[3]);
    if (args[5].compare("NO_REACH_TABLE")) {
        cerr << "Building reachability table..." << endl;
        ReachabilityTable rt;
        rt.build(pg);
        if (!rt.writeToFile(args[5])) { // rt file
            delete pg;
            cerr << "failed to write reachability table file" << endl;
            exit(1);
        }
        cerr << "Finished building reachability table" << endl;
    }
    cerr << "Start building CPD..." << endl;
    int first_node_idx, last_node_idx;
    if (this->nr_args == 8) {
        first_node_idx = atoi(args[6].c_str());
        last_node_idx = atoi(args[7].c_str());
    } else {
        assert (nr_args == 6);
        first_node_idx = 0;
        last_node_idx = pg->getNrNodes() - 1;
    }
    CompressedMoveDatabase cmd(pg, first_node_idx, last_node_idx);
    cmd.build(list_trimming);
    time(&end);
    double t = difftime(end, start);
    int nr_nodes = last_node_idx - first_node_idx + 1;
    print_preprocessing_stats(cmd, t, args[2], nr_nodes);
    ofstream outfile;
    //cerr << "Done after " << t << endl;
    cerr << "Opening cpd output file " << args[4] << endl; // cpd file
    outfile.open(args[4].c_str());
    if (!outfile) {
        cerr << "Unable to open " << args[4] << endl;
        delete pg;
        exit (1);
    }
    cmd.writeDataToStream(outfile);
    delete pg;
}

void ParamManager::build_qt_rm() const {
    cerr << "files: " << args[2] << "; " << args[3] << endl;
    time_t start, end;
    time(&start);
    AbstractGraph * pg = new GenericMap(args[2], args[3]);
    cerr << "Start building quad trees..." << endl;
    int first_node_idx, last_node_idx;
    if (this->nr_args == 6) {
        first_node_idx = atoi(args[6].c_str());
        last_node_idx = atoi(args[7].c_str());
    } else {
        assert (nr_args == 4);
        first_node_idx = 0;
        last_node_idx = pg->getNrNodes() - 1;
    }
    QTDatabase qtd(pg, first_node_idx, last_node_idx);
    qtd.build();
    time(&end);
    double t = difftime(end, start);
    print_build_qt_stats(pg, qtd, t, args[2], last_node_idx - first_node_idx + 1);
    delete pg;
}

void ParamManager::call_appropriate_method() const {
    if (is_mode_preprocessing_gridmap()) {
        preprocess_gm();
    } else if (is_mode_pathfinding()) {
        find_paths_gm();
    } else if (is_mode_printing()) {
        // old stuff specific to grids
    } else if (is_mode_preprocessing_roadmap()) {
        // build CPD with rectangle decomposition + list trimming
        preprocess_rm(true);
    } else if (is_mode_build_cpd_rd()) {
        // build CPD with rectangle decomposition only (no list trimming)
        preprocess_rm(false);
    } else if (is_mode_pathfinding_rm()) {
        find_paths_rm();
    } else if (is_mode_build_qt_roadmap()) {
        build_qt_rm();
    } else if (is_mode_build_qt_gridmap()) {
        build_qt_gm();
    } else if (is_mode_pf_comp()) {
        find_paths_comp();
    } else if (is_mode_preproc_comp()) {
        preprocess_comp();
    } else if (is_mode_pf_cpd2()) {
        find_paths_rm();
    }
}

void ParamManager::print_preprocessing_stats(const CompressedMoveDatabase & cmd,
        double t, const string & filename, int nr_nodes) const {
    cout << endl << "SUMMARY FOR " << filename << endl;
    cout << " -- Pre-processing time in seconds: " << t << endl;
    cout << " -- Nr locations: " << nr_nodes << endl;
    cout << " -- Nr rectangles: " << cmd.getNrRectangles() << endl;
    cout << " -- Nr rectangles before default move filtering: " << 
            cmd.getNrRectanglesBeforeDeMoFiltering() << endl;
    cout << " -- Nr 1x1 rectangles: " << cmd.getNr1x1Rectangles() << endl;
    cout << " -- Avg rectangles per list: " << setprecision(3) << 
            1.0*cmd.getNrRectangles()/nr_nodes << endl;
    cout << cmd;
    cout << endl;
}

void ParamManager::print_build_qt_stats(const AbstractGraph* graph,
        const QTDatabase & qtd, double t, const string & filename, int nr_nodes) const {
    cout << endl << "Summary for " << filename << endl;
    cout << "Time to build quadtree database in seconds: " << t << endl;
    cout << "Nr locations: " << nr_nodes << endl;
    cout << "Nr quadtree blocks: " << qtd.getNrBlocks() << endl;
    cerr << "Avg checks in qt: " << qtd.getNrAverageChecks() << endl;
    //cout << qtd;
    cout << endl;
}

void ParamManager::build_qt_gm() const {
    time_t start, end;
    time(&start);
    AbstractGraph * pg = new GridMap(args[2].c_str());
    cerr << "Start building quad trees..." << endl;
    int first_node_idx, last_node_idx;
    if (this->nr_args == 5) {
        first_node_idx = atoi(args[3].c_str());
        last_node_idx = atoi(args[4].c_str());
    } else {
        assert (nr_args == 3);
        first_node_idx = 0;
        last_node_idx = pg->getNrNodes() - 1;
    }
    QTDatabase qtd(pg, first_node_idx, last_node_idx);
    qtd.build();
    time(&end);
    double t = difftime(end, start);
    print_build_qt_stats(pg, qtd, t, args[2], last_node_idx - first_node_idx + 1);
    delete pg;
}

// build cpd for grid maps
void ParamManager::preprocess_gm() const {
    time_t start, end;
    time(&start);
    AbstractGraph * pg = new GridMap(args[2].c_str());
    if (args[3].compare("NO_REACH_TABLE")) {
        ReachabilityTable rt;
        rt.build(pg);
        if (!rt.writeToFile(args[3])) { // rt file
            delete pg;
            cerr << "failed to write reachability table file" << endl;
            exit(1);
        }
        cerr << "Finished building reachability table" << endl;
    }
    CompressedMoveDatabase cmd(pg);
    cmd.build(false);
    time(&end);
    double t = difftime(end, start);
    print_preprocessing_stats(cmd, t, args[2], pg->getNrNodes());
    ofstream outfile;
    cerr << "Opening output file " << args[3] << endl;
    outfile.open(args[3].c_str());
    if (!outfile) {
        cerr << "Unable to open " << args[3] << endl;
        exit (1);
    }
    cmd.writeDataToStream(outfile);
    delete pg;
}

void ParamManager::find_paths_gm() const {
    ifstream in_cpd_file;
    ofstream outfile;
    outfile.open(args[6].c_str()); // results file
    if (!outfile) {
        cerr << "Unable to open " << args[6] << endl;
        exit (1);
    }
    AbstractGraph * pg = new GridMap(args[2].c_str());
    ReachabilityTable rt;
    cerr << "reading reachability table..." << endl;
    if (!rt.readFromFile(args[4])) { // rt file
        delete pg;
        cerr << "failed to write reachability table file" << endl;
        exit(1);
    }
    CompressedMoveDatabase cmd;
    cerr << "reading cpd..." << endl;
    in_cpd_file.open(args[3].c_str()); // cpd file
    if (!in_cpd_file) {
        cerr << "Unable to open " << args[3] << endl;
        exit (1);
    }
    cmd.readDataFromStream2(in_cpd_file);
    in_cpd_file.close();
    cerr << "reading scenario..." << endl;
    ScenarioRM scen;
    scen.readFromFile(args[5]); // scenario file
    cerr << "running pathfinding..." << endl;
    int nr_queries = scen.getNrRecords();
    // outfile << nr_queries << endl;
    clock_t global_clock = clock();
    double global_length = 0;
    int global_nr_moves = 0;
    int global_nr_rect_checks = 0;
    int nr_rect_checks;
    for (int i = 0; i < nr_queries; i++) {
        RecordRM rec = scen.getRecord(i);
        clock_t clo = clock();
        double length = -1;
        int nr_moves = -1;
        const int NR_RUNS = 10000;
        double dif = 0;
        clock_t clocks = 0;
        if (rt.pathExists(rec.start_idx, rec.target_idx)) {
            for (int x = 0; x < NR_RUNS; x++)
                cmd.findPath(rec.start_idx, rec.target_idx,
                    length, nr_moves, nr_rect_checks, pg);
            clocks = clock() - clo;
            dif = ((double)clocks) / CLOCKS_PER_SEC;
            dif /= NR_RUNS;
        }
        global_length += length;
        global_nr_moves += nr_moves;
        global_nr_rect_checks += nr_rect_checks;
        outfile << pg->getNode(rec.start_idx)->getStringId() << " "
                << pg->getNode(rec.target_idx)->getStringId() << " ";
        outfile << length << " ";
        outfile << nr_moves << " " << dif << " " << clocks << endl;
    }
    double global_dif = ((double)clock() - global_clock) / CLOCKS_PER_SEC;
    cerr << "Stats for " << nr_queries << " queries:" << endl;
    cerr << " -- total # moves: " << global_nr_moves << endl;
    cerr << " -- total cost: " << global_length << endl;
    cerr << " -- total time (seconds): " << global_dif << endl;
    cerr << " -- average time per query (seconds): " << global_dif/nr_queries << endl;
    cerr << " -- total # rectangle checks: " << global_nr_rect_checks << endl;
    cerr << " -- average # rectangle checks per node: " <<
            ((double)global_nr_rect_checks)/global_nr_moves << endl;
    outfile.close();
}

void ParamManager::find_paths_rm() const {
    ifstream in_cpd_file;
    ofstream outfile;
    //char *vertexfile, char* edgefile, char* cpdfile, char* rtfile, char* scenariofile, char* resultsfile
    //args[2], args[3], args[4], args[5], args[6], args[7]
    //cerr << "Opening input file " << cpdfile << endl;
    outfile.open(args[7].c_str()); // results file
    if (!outfile) {
        cerr << "Unable to open " << args[7] << endl;
        exit (1);
    }
    AbstractGraph * pg = new GenericMap(args[2], args[3]);
    ReachabilityTable rt;
    cerr << "reading reachability table..." << endl;
    if (!rt.readFromFile(args[5])) { // rt file
        delete pg;
        cerr << "failed to read reachability table file" << endl;
        exit(1);
    }
    AbstractCPD *cpd;
    assert (mode == 8 || mode == 5);
    if (this->mode == 8)
        cpd = new CPD2();
    else
        cpd = new CompressedMoveDatabase();
    cerr << "reading cpd..." << endl;
    in_cpd_file.open(args[4].c_str(), ios::binary); // cpd file
    if (!in_cpd_file) {
        cerr << "Unable to open " << args[4] << endl;
        exit (1);
    }
    cpd->readDataFromStream2(in_cpd_file);
    in_cpd_file.close();
    cerr << "reading scenario..." << endl;
    ScenarioRM scen;
    scen.readFromFile(args[6]); // scenario file
    cerr << "running pathfinding..." << endl;
    int nr_queries = scen.getNrRecords();
	// increase NR_RUNS for a better timing accurracy
	const int NR_RUNS = 1;
	cerr << "each query will be executed "<<NR_RUNS<<"times"<<endl;

	// setup a progress indicator
	const int pctTick = 5;
	cerr << "Progress: [";
	for(int i=pctTick; i <= 100; i+=pctTick)
	{
		cerr << " ";
	}
	cerr << "]" << flush;

    // outfile << nr_queries << endl;
    clock_t global_clock = clock();
    double global_length = 0;
    long global_nr_moves = 0;
    long global_nr_rect_checks = 0;
    int nr_rect_checks = 0;
    for (int i = 0; i < nr_queries; i++) {
        RecordRM rec = scen.getRecord(i);
        clock_t clo = clock();
        double length = -1;
        int nr_moves = -1;
        double dif = 0;
        clock_t clocks = 0;
        //cerr << "start: " << rec.start_idx << endl;
        //cerr << "target: " << (rec.target_idx) << endl;
        if (rt.pathExists(rec.start_idx, rec.target_idx)) {
            for (int x = 0; x < NR_RUNS; x++)
                cpd->findPath(rec.start_idx, rec.target_idx, 
                    length, nr_moves, nr_rect_checks, pg);
            clocks = clock() - clo;
            dif = ((double)clocks) / CLOCKS_PER_SEC;
            dif /= NR_RUNS;
        }
        global_length += length;
        global_nr_moves += nr_moves;
        global_nr_rect_checks += nr_rect_checks;

        //if (i % 1000 == 0)
        //    cerr << "computed " << i << " paths" << endl;
        ////cerr << "Nr clocks for 10000 identical runs: " << clocks << endl;
        ////cerr << "Time (sec): " << dif << endl;
        ////cerr << "Path cost:" << length * 0.01 << endl;
        ////cerr << "Nr moves on path: " << nr_moves << endl;
        outfile << pg->getNode(rec.start_idx)->getStringId() << " "
                << pg->getNode(rec.target_idx)->getStringId() << " ";
        outfile << length << " ";
        outfile << nr_moves << " " << dif << " " << clocks << endl;

		// update progress indicator
		int pctDone = ((double)i/(double)nr_queries)*100.;
		if(pctDone == 0 || (pctDone % pctTick) != 0)
			continue;
		cerr << "\rProgress: ["; 
		for(int i=pctTick; i <= 100; i+=pctTick)
		{
			if(i < pctDone)
			{
				cerr << "-";
			}
			else if(i == pctDone)
			{
				cerr << ">";
			}
			else
			{
				cerr << " ";
			}
		}
		cerr << "]"<<flush;
    }
    double global_dif = ((double)clock() - global_clock) / CLOCKS_PER_SEC;
    cerr << "SUMMARY OF RESULTS" << endl;
    cerr << " -- cpd file: " << args[4] << endl;
    cerr << " -- scenario file: " << args[6] << endl;
    cerr << " -- # queries: " << nr_queries << endl;
    cerr << " -- total # edges: " << global_nr_moves << endl;
    cerr << " -- average # edges per query: "
            << setprecision(5) << 1.0*global_nr_moves/nr_queries << endl;
    cerr << " -- total cost: " << global_length << endl;
    cerr << " -- total time (seconds): " << global_dif << endl;
    cerr << " -- average time per query (milliseconds): " << 
            setprecision (5) << 1000*global_dif/(nr_queries) << endl;
    cerr << " -- average time per move (microseconds): " <<
            setprecision (5) << 1000000*global_dif/(global_nr_moves) << endl;
    cerr << " -- total # rectangle checks: " << global_nr_rect_checks << endl;
    cerr << " -- average # rectangle checks per node: " <<
            ((double)global_nr_rect_checks)/global_nr_moves << endl;
    outfile.close();
    delete cpd;
}

void ParamManager::find_paths_comp() const {
    ifstream in_cpd_file;
    ofstream outfile;
    outfile.open(args[5].c_str()); // results file
    if (!outfile) {
        cerr << "Unable to open " << args[5] << endl;
        exit (1);
    }
    GridMap * pg = new GridMap(args[2].c_str());
    assert (mode == 11);
    CompressedMoveDatabase cpd;
    cerr << "reading cpd..." << endl;
    in_cpd_file.open(args[3].c_str(), ios::binary); // cpd file
    if (!in_cpd_file) {
        cerr << "Unable to open " << args[3] << endl;
        exit (1);
    }
    cpd.readDataBinary(in_cpd_file);
    vector<bool> bm = pg->getBitMap();
    MapCpd mapcpd(bm, pg->getWidth(), pg->getHeight(), args[3].c_str());
    in_cpd_file.close();
    cerr << "reading scenario..." << endl;
    Scenario scen;
    scen.readFromFile(args[4].c_str()); // scenario file
    cerr << "running pathfinding..." << endl;
    int nr_queries = scen.getNrRecords();
	// increase NR_RUNS for a better timing accurracy
	const int NR_RUNS = 10000;
	cerr << "each query will be executed " << NR_RUNS << " times";
        cerr << " for a more accurate time measurement" << endl;
        if (NR_RUNS > 1)
            cerr << "(this is why it takes longer than one might expect)" << endl;

    clock_t global_clock = clock();
    double global_length = 0;
    long global_nr_moves = 0;
    long global_nr_rect_checks = 0;
    int nr_rect_checks = 0;
    for (int i = 0; i < nr_queries; i++) {
        Record rec = scen.getRecord(i);
        xyLoc s, g;
        s.x = rec.start_col;
        s.y = rec.start_row;
        g.x = rec.target_col;
        g.y = rec.target_row;
        AbstractGraphNode * pns = pg->getNode2(s.x, s.y);
        if (pns == NULL)
            continue;
        AbstractGraphNode * png = pg->getNode2(g.x, g.y);
        if (png == NULL)
            continue;
        //int start_idx = pg->getNode2(s.x, s.y)->getGlobalIdx();
        //int target_idx = pg->getNode2(g.x, g.y)->getGlobalIdx();
        clock_t clo = clock();
        double length = 0;
        int nr_moves = -1;
        double dif = 0;
        clock_t clocks = 0;
        vector<xyLoc> path;
        if (true) {
            for (int x = 0; x < NR_RUNS; x++) {
                //cpd.findPath(start_idx, target_idx, 
                //    length, nr_moves, nr_rect_checks, pg);
                bool finished = false;
                path.clear();
                while (!finished) {
                    finished = GetPath(&mapcpd, s, g, path);
                    if (!finished)
                        assert (path.size() <= 21);
                }
                //assert (nr_moves == path.size() - 1);
            }
            clocks = clock() - clo;
            dif = ((double)clocks) / CLOCKS_PER_SEC;
            dif /= NR_RUNS;
            nr_moves = path.size() - 1;
            for (int k = 1; k < path.size(); k++) {
                if (path[k-1].x == path[k].x || path[k-1].y == path[k].y)
                    length += ONE;
                else {
                    length += SQRT2;
                }
            }
        }
        global_length += length;
        global_nr_moves += nr_moves;
        global_nr_rect_checks += nr_rect_checks;

        outfile << s.x << " " << s.y << " " << g.x << " " << g.y << " ";
        outfile << length << " ";
        outfile << nr_moves << " " << dif << " " << clocks << endl;

    }
    double global_dif = ((double)clock() - global_clock) / CLOCKS_PER_SEC;
    global_dif /= NR_RUNS;
    cerr << "SUMMARY OF RESULTS" << endl;
    cerr << " -- cpd file: " << args[3] << endl;
    cerr << " -- scenario file: " << args[4] << endl;
    cerr << " -- # queries: " << nr_queries << endl;
    cerr << " -- total # edges: " << global_nr_moves << endl;
    cerr << " -- average # edges per query: "
            << setprecision(5) << 1.0*global_nr_moves/nr_queries << endl;
    cerr << " -- total cost: " << global_length << endl;
    cerr << " -- total time (seconds): " << global_dif << endl;
    cerr << " -- average time per query (milliseconds): " << 
            setprecision (5) << 1000*global_dif/(nr_queries) << endl;
    cerr << " -- average time per move (microseconds): " <<
            setprecision (5) << 1000000*global_dif/(global_nr_moves) << endl;
    cerr << " -- total # rectangle checks: " << global_nr_rect_checks << endl;
    cerr << " -- average # rectangle checks per node: " <<
            ((double)global_nr_rect_checks)/global_nr_moves << endl;
    outfile.close();
}

void ParamManager::preprocess_comp() const {
    cerr << "files: " << args[2] << "; " << args[3] << endl;
    time_t start, end;
    time(&start);
    GridMap gm(args[2].c_str());
    vector<bool> vb = gm.getBitMap();
    PreprocessMap(vb, gm.getWidth(), gm.getHeight(), args[3].c_str());
    time(&end);
    double t = difftime(end, start);
    //int nr_nodes = last_node_idx - first_node_idx + 1;
    //print_preprocessing_stats(&gm, cmd, t, args[2], nr_nodes);
}
