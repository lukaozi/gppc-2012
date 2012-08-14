#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <cstring>
#include <string>

using std::string;

struct xyLoc {
  int16_t x;
  int16_t y;
};

class node_item{
public:
	int area;
	xyLoc position;
};

class node{
public:
	node();
	int id, depth, parent;
	double total_distance, distance;
	bool visited, temp;
	std::vector<int> areas;
	std::vector<node_item> items;
	std::vector<node> children;
};


class map_area{
public:
	map_area(int id);
	void addPoints(int row, int start, int end);
	bool can_go_left();
	bool can_go_right();
	bool contains(xyLoc point);
	int id, last_left, last_right, blast_left, blast_right, last_width, start_row, row_count;
	bool to_close, open_left, close_left, open_right, close_right;
	std::vector<int> start_points, end_points, nodes;
	std::vector<xyLoc> internalNodePoints;
};
class row_slice
{
public:
	row_slice();
	row_slice(int y);
	void set_first(int x);
	void set_last(int x);
	void append_points(int end);
	void merge_slice(row_slice to_merge);
	std::vector<int> ids;
	int first, last, row, area_inserted, current_front, area_connected, size;
	bool inserted;
private:
	std::vector<xyLoc> points;

};

// Entry Initial Functions
const char *GetName();
void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path);
void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename);
void GetSuccessors(node s, xyLoc g, std::vector<node> &neighbors);

// Personal Functions
void SetFirstSearch(bool status);
void InitializeSearch();
bool get_str_path(std::vector<bool> map, xyLoc start, xyLoc end, std::vector<xyLoc> &path, int &r_distance);
bool get_str_path(std::vector<bool> map, xyLoc start, xyLoc end, map_area area, std::vector<xyLoc> &path, int &r_distance);
void export_map(std::vector<map_area> areas, std::vector<node> nodes, const char* filename);


// Map Visualizer - Functions to help visualize the result (For Debugging only...)
void write_file(const char *fname, std::vector<string> tempMap, int width, int height);
void write_path(std::vector<string> &tempMap, int width, int height, std::vector<xyLoc> path);
void write_map(std::vector<bool> map, int width, int height, xyLoc start, xyLoc end, std::vector<xyLoc> path);
void write_map(std::vector<bool> map, int width, int height, std::vector<map_area> areas, std::vector<node> nodes);
void write_file(const char *fname, std::vector<string> tempMap, int width, int height);
void write_path(std::vector<string> &tempMap, int width, int height, std::vector<xyLoc> path);
void obscure_area(std::vector<string> &tempMap,int width,int height, xyLoc point);
void write_map_path(std::vector<bool> map, int width, int height, xyLoc sp, xyLoc gp, std::vector<xyLoc> path);

