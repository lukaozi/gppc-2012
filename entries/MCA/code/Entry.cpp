#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <math.h>
#include <cstring>
#include <string>

#include "Entry.h"

using std::string;
using std::ofstream;

std::vector<bool> map;
std::vector<node> succ;
int width, height;
std::vector<map_area> final_areas;
std::vector<node> final_nodes;
std::vector<node> p_nodes;
bool first_search = true;
int real_distance;

bool sort_area_by_left (map_area f, map_area k);
int GetIndex(xyLoc s);
double getAbsDistance(xyLoc s, xyLoc g);
void ExtractPath(xyLoc end, std::vector<xyLoc> &finalPath);
void ExtractPath(xyLoc goal, node end, std::vector<xyLoc> &finalPath);
bool sort_nodes_by_id(node f, node l) { return f.id < l.id; }


void SetFirstSearch(bool status)
{
	first_search = status;
}

void InitializeSearch()
{
	p_nodes = final_nodes;
}

const char *GetName()
{
	return "GraphAreas";
}

/*
 * Function to search into the front
 * Using a depth first search
 * Heuristic: Manhattan distance from the end
 *
 */
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	node current;

	// Clear successors if first search
	if (first_search)
		succ.clear();

	// Insert start and goal nodes if front is empty and we begin search
	if (succ.size() == 0 && first_search)
	{
		first_search = false;

		// Initialize and clear variables to start searching.
		InitializeSearch();

		// Check for nodes
		if (p_nodes.size() == 0)
		{
			printf("No problem set.\n");
			return true;
		}

		// Try to go directly from start to finish
		bool dn = get_str_path(map, s, g, path, real_distance);
		if (dn)
			return true;
		// Try to go in reverse directly
		bool dn1 = get_str_path(map, g, s, path, real_distance);
		if (dn1)
		{
			std::reverse(path.begin(), path.end());
			return true;
		}
		// There is no direct path, continue searching
		// Clear path
		path.clear();

		// Get areas containing start and goal positions
		int s_area, g_area;
		s_area = g_area = -1;

		for (unsigned i=0; i<final_areas.size(); i++)
		{
			if (s_area == -1 && final_areas[i].contains(s))
				s_area = final_areas[i].id;
			if (g_area == -1 && final_areas[i].contains(g))
				g_area = final_areas[i].id;

			if (s_area != -1 && g_area != -1)
				break;
		}

		// If a point is not valid or not found, stop the search
		if (s_area == -1 || g_area == -1)
			return true;

		//_____ Create node for start position
		node s_node;
		s_node.id = p_nodes.size()-2;
		s_node.parent = -1;
		s_node.visited = true;

		node_item t1;
		t1.area = s_area;
		t1.position.y = s.y;
		t1.position.x = s.x;
		s_node.items.push_back(t1);
		s_node.areas.push_back(s_area);

		t1.area = s_area;
		t1.position.y = s.y;
		t1.position.x = s.x;
		s_node.items.push_back(t1);
		s_node.areas.push_back(s_area);
		p_nodes[p_nodes.size()-2] = s_node;

		//_____ Create node for goal position
		node g_node;
		g_node.id = p_nodes.size()-1;
		g_node.parent = -1;
		g_node.temp = true;

		node_item t2;
		t2.area = g_area;
		t2.position.y = g.y;
		t2.position.x = g.x;
		g_node.items.push_back(t2);
		g_node.areas.push_back(g_area);

		t2.area = g_area;
		t2.position.y = g.y;
		t2.position.x = g.x;
		g_node.items.push_back(t2);
		g_node.areas.push_back(g_area);
		p_nodes[p_nodes.size()-1] = g_node;

		// Check if there is a straight path between start and goal first (if nodes are in the same area)
		if (s_area == g_area)
		{
			get_str_path(map, s_node.items[0].position, g_node.items[0].position, final_areas[s_area], path, real_distance);
			return true;
		}

		// Create children of start point-node
		for (unsigned i=0; i<final_areas[s_area].nodes.size(); i++)
		{
			node temp = p_nodes[final_areas[s_area].nodes[i]];
			bool ch_match = (s_area == temp.areas[0] || s_area == temp.areas[1]);
			if (s_node.id != temp.id && ch_match)
			{
				xyLoc end = ((s_area == temp.areas[0]) ? temp.items[0].position : temp.items[1].position);
				std::vector<xyLoc> mhPath;
				temp.distance = getAbsDistance(s,end);
				temp.temp = true;
				s_node.children.push_back(temp);
			}
		}
		// Create parents of goal point-node
		for (unsigned i=0; i<final_areas[g_area].nodes.size(); i++)
		{
			node temp = p_nodes[final_areas[g_area].nodes[i]];
			bool match = (g_area == temp.areas[0] || g_area == temp.areas[1]);
			if (match)
			{
				xyLoc end = ((s_area == temp.areas[0]) ? temp.items[0].position : temp.items[1].position);
				std::vector<xyLoc> mhPath;
				temp.distance = getAbsDistance(s,end);
				p_nodes[temp.id].children.push_back(g_node);
			}
		}

		current = s_node;
	}

	int min = 500000;
	int indx;
	// Search if goal is in the front and get minimum distance in the meantime
	for (unsigned i=0; i<succ.size(); i++)
	{
		if ((g.x == succ[i].items[0].position.x && g.y == succ[i].items[0].position.y) || (g.x == succ[i].items[1].position.x && g.y == succ[i].items[1].position.y))
		{
			ExtractPath(g, succ[i], path);
			return true;
		}
		// Get minimum distance in order to choose the next node later
		if (succ[i].total_distance < min)
		{
			min = succ[i].total_distance;
			indx = i;
		}
	}
	// If there are nodes in the front, choose the next node (the index is given above)
	if (succ.size() > 0)
	{
		current = succ[indx];
		p_nodes[current.id].visited = true;
		succ.erase(succ.begin() + indx);
	}

	// Create successors
	GetSuccessors(current, g, succ);

	// If successors are empty, stop the search
	if (succ.size() == 0)
		return true;

	return false;
}

double getAbsDistance(xyLoc s, xyLoc g)
{
	double sqr2 = sqrt(2)-1;
	return (abs(s.x - g.x) > abs(s.y - g.y) ? abs(s.x - g.x) + abs(s.y - g.y)*sqr2 : abs(s.y - g.y) + abs(s.x - g.x)*sqr2);
}

int GetIndex(xyLoc s)
{
	return s.y*width+s.x;
}

void GetSuccessors(node s, xyLoc g, std::vector<node> &cfront)
{
	int sqr2 = sqrt(2)-1;

	// Add connected nodes that are not already in
	for (unsigned i=0; i<s.children.size(); i++)
	{
		node k = s.children[i];
		int pos = -1;
		// Look for node in the front
		for (unsigned j=0; j<cfront.size(); j++)
			if (k.id == cfront[j].id)
				pos = j;

		// - Calculate distance to goal (heuristic)
		double dtg1 = (abs(k.items[0].position.x - g.x) > abs(k.items[0].position.y - g.y) ? abs(k.items[0].position.x - g.x) + abs(k.items[0].position.y - g.y)*sqr2 : abs(k.items[0].position.y - g.y) + abs(k.items[0].position.x - g.x)*sqr2);
		double dtg2 = (abs(k.items[1].position.x - g.x) > abs(k.items[1].position.y - g.y) ? abs(k.items[1].position.x - g.x) + abs(k.items[1].position.y - g.y)*sqr2 : abs(k.items[1].position.y - g.y) + abs(k.items[1].position.x - g.x)*sqr2);
		double dtg = (dtg1 < dtg2 ? dtg1 : dtg2);

		if ((pos == -1 && !p_nodes[k.id].visited))
		{
			// If node not in the front and not yet visited, insert it
			int f_id = k.id;
			p_nodes[f_id].depth = s.depth + 1;
			// Calculate node score
			p_nodes[f_id].total_distance = s.total_distance + k.distance + dtg;
			// Set Parent
			p_nodes[f_id].parent = s.id;
			cfront.push_back(p_nodes[f_id]);
		}
		else if (pos > 0 && cfront.size() > 0 && ((s.total_distance + k.distance + dtg) < cfront[pos].total_distance))
		{
			/*
			 * If node is in the front but total distance is smaller than before,
			 * recalculate score and change parent.
			 *
			 */
			cfront[pos].depth = s.depth + 1;
			cfront[pos].total_distance = s.total_distance + k.distance + dtg;
			cfront[pos].parent = s.id;
			p_nodes[cfront[pos].id].parent = s.id;
		}
	}
}

/*
 * Extracts the Path starting from the last node (goal)
 * and going in reverse.
 *
 */
void ExtractPath(xyLoc goal, node end, std::vector<xyLoc> &finalPath)
{
	//printf("extracting path...\n");
	node current = end;
	std::vector<xyLoc> path;
	// Get the goal area
	int goal_area = (current.items[0].position.x == goal.x && current.items[0].position.y == goal.y ? current.areas[0] : current.areas[1]);
	// Make path from one node_item to another if node_item is on different area
	int start_count_area = ((current.areas[0] == p_nodes[current.parent].areas[0] || current.areas[0] == p_nodes[current.parent].areas[1]) ? current.areas[0] : current.areas[1]);
	if (start_count_area != goal_area)
	{
		// Get the end position of the path
		int end_position = ((current.areas[0] == p_nodes[current.parent].areas[0] || current.areas[0] == p_nodes[current.parent].areas[1]) ? 0 : 1);
		get_str_path(map, goal, current.items[end_position].position, path, real_distance);
		// Add path to finalPath
		for (unsigned i=0; i<path.size(); i++)
			finalPath.push_back(path[i]);
	}

	while (current.parent != -1)
	{
		//printf("inside extract while...\n");
		node next = p_nodes[current.parent];
		//printf("areas: [%d,%d], (%d,%d)\n",current.areas[0],current.areas[1],next.areas[0],next.areas[1]);
		if (current.areas[0] == next.areas[0] || current.areas[0] == next.areas[1])
		{
			xyLoc s,g;
			s = current.items[0].position;
			g = (current.areas[0] == next.areas[0] ? next.items[0].position : next.items[1].position);
			// Get Path from one node to another
			get_str_path(map, s, g, final_areas[current.areas[0]], path, real_distance);
			xyLoc g1 = (current.areas[0] == next.areas[0] ? next.items[1].position : next.items[0].position);
			std::vector<xyLoc> path1;
			// Get path between the two points of the node
			get_str_path(map, g, g1, path1, real_distance);
			for (unsigned i=0; i<path1.size(); i++)
				path.push_back(path1[i]);
		}
		else
		{
			xyLoc s,g;
			s = current.items[1].position;
			g = (current.areas[1] == next.areas[0] ? next.items[0].position : next.items[1].position);
			// Get Path from one node to another
			get_str_path(map, s, g, final_areas[current.areas[1]], path, real_distance);
			xyLoc g1 = (current.areas[1] == next.areas[0] ? next.items[1].position : next.items[0].position);
			std::vector<xyLoc> path1;
			// Get path between the two points of the node
			get_str_path(map, g, g1, path1, real_distance);
			for (unsigned i=0; i<path1.size(); i++)
				path.push_back(path1[i]);
		}

		// Add path to finalPath
		for (unsigned i=0; i<path.size(); i++)
			finalPath.push_back(path[i]);
		current = next;
	}
	// Reverse path because it's been extracted from the goal to start
	std::reverse(finalPath.begin(), finalPath.end());
}

void ExtractPath(xyLoc end, std::vector<xyLoc> &finalPath)
{
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	void *reference;
	map = bits;
	width = w;
	height = h;

	// If graph is already in memory, don't re-calculate
	if (final_nodes.size() > 0)
		return reference;


	printf("Preparing map for search...\n");
	int size;

	FILE *f;
	f = fopen(filename, "r");
	printf("Loading nodes...\n");
	if (f)
	{
		// Scan area count
		fscanf(f, ":areas %d\n", &size);
		final_areas.clear();
		for (int i=0; i<size; i++)
		{
			// Read area
			int id, start_row, row_count;
			fscanf(f, ":area %d|%d,%d\n", &id, &start_row, &row_count);
			map_area area(id);
			area.start_row = start_row;
			area.row_count = row_count;
			// Read area's start and end points
			for (int j=0; j<row_count; j++)
			{
				int start,end;
				fscanf(f, "%d-%d|", &start, &end);
				area.start_points.push_back(start);
				area.end_points.push_back(end);
			}
			fscanf(f, "\n");
			// Insert area into vector
			final_areas.push_back(area);
		}
		fscanf(f, "\n");


		// Scan node count
		int node_count;
		fscanf(f, ":nodes %d\n", &node_count);
		final_nodes.clear();
		for (int i=0; i<node_count; i++)
		{
			int id,a1,a2,t1x,t1y,t2x,t2y;
			fscanf(f, ":node %d=%d:%d,%d|%d:%d,%d\n", &id, &a1,&t1x,&t1y,&a2,&t2x,&t2y);

			node current_node;
			current_node.id = id;

			node_item t1;
			t1.area = a1;
			t1.position.y =  t1y;
			t1.position.x = t1x;
			current_node.items.push_back(t1);
			current_node.areas.push_back(a1);

			t1.area = a2;
			t1.position.y = t2y;
			t1.position.x = t2x;
			current_node.items.push_back(t1);
			current_node.areas.push_back(a2);


			// Scan children
			int ch_count;
			fscanf(f, ":children %d\n", &ch_count);
			for (int j=0; j<ch_count; j++)
			{
				int c_id, dist;
				fscanf(f, "%d:%d|", &c_id, &dist);
				node child;
				child.id = c_id;
				child.distance = dist;
				current_node.children.push_back(child);
			}
			final_nodes.push_back(current_node);
			fscanf(f, "\n");
		}

		std::vector<node> temp_nodes = final_nodes;
		final_nodes.clear();
		// Reposition children
		for (unsigned i=0; i<temp_nodes.size(); i++)
		{
			node current = temp_nodes[i];
			for (unsigned j=0; j<current.children.size(); j++)
			{
				int ch_id = current.children[j].id;
				int dist = current.children[j].distance;
				current.children[j] = temp_nodes[ch_id];
				current.children[j].distance = dist;
			}
			final_nodes.push_back(current);
		}
	}

	// Resize final_nodes in order to include starting and goal nodes
	final_nodes.resize(final_nodes.size() + 2);

	printf("Preparation finished...\n");
	printf("Number of areas: %d\n", final_areas.size());
	printf("Number of nodes: %d\n", final_nodes.size());

	return reference;
}

// Calculates the path and the real distance between two points within a Manhattan-cohesion area
bool get_str_path(std::vector<bool> map, xyLoc start, xyLoc end, std::vector<xyLoc> &path, int &r_distance)
{
	double d_distance = 0;

	// Store path to vector
	path.clear();

	xyLoc next;
	next.x = start.x;
	next.y = start.y;

	// Calculate step in each axis
	int factor_x = (start.x > end.x ? -1 : 1);
	int factor_y = (start.y > end.y ? -1 : 1);

	path.push_back(next);

	// While not there yet
	while (abs(next.x - end.x) > 0 || abs(next.y - end.y) > 0)
	{
		/*
		 * Store last position into a variable
		 * in order to compare it with next position and avoid walls
		 *
		 */
		xyLoc last;
		last.x = next.x;
		last.y = next.y;

		// Calculate distance in each axis
		int d_x = abs(next.x - end.x);
		int d_y = abs(next.y - end.y);
		// Diagonal is preferred than cardinal
		// If diagonal is possible, move diagonal
		if (d_x > 0 && d_y > 0)
		{
			next.x += factor_x;
			next.y += factor_y;
		}
		else if (d_x > 0)
			next.x += factor_x;
		else
			next.y += factor_y;

		// If Next is not a valid position or is diagonal and cardinal are not valid
		if (!map[next.y*width+next.x] || (abs(next.x - last.x) && abs(next.y - last.y)))
		{
			// If is a cardinal move and there is a wall, no path
			if (!map[next.y*width+next.x] && !(abs(next.x - last.x) && abs(next.y - last.y)))
				return false;
			if (!map[last.y*width+next.x])
			{
				// If vertical is not a valid position, step horizontal
				next.x = last.x;
				next.y = last.y + factor_y;
				d_distance++;
			}
			else if (!map[next.y*width+last.x])
			{
				// If horizontal is not a valid position, step vertical
				next.y = last.y;
				next.x = last.x + factor_x;
				d_distance++;
			}
			else	// Else, it's a diagonal move, add sqrt(2) to real_distance
				d_distance += 1.4142;

			// At last, if move is not valid again, no path
			if (!map[next.y*width+next.x])
				return false;
		}
		else
			d_distance++;

		path.push_back(next);
	}

	// Round up real distance
	r_distance = (ceil(d_distance) - d_distance > 0.5 ? floor(d_distance) : ceil(d_distance));

	// Path is finished, return it
	return true;
}

/*
 * Calculates the path between two points within a Manhattan-cohesion area.
 * The area is given in order to maintain within its boundaries.
 *
 */
bool get_str_path(std::vector<bool> map, xyLoc start, xyLoc end, map_area area, std::vector<xyLoc> &path, int &r_distance)
{
	double d_distance = 0;

	// Store path to vector
	path.clear();

	xyLoc next;
	next.x = start.x;
	next.y = start.y;

	// Calculate step in each axis
	int factor_x = (start.x > end.x ? -1 : 1);
	int factor_y = (start.y > end.y ? -1 : 1);

	path.push_back(next);
	// While not there yet
	while (abs(next.x - end.x) > 0 || abs(next.y - end.y) > 0)
	{
		/*
		 * Store last position into a variable
		 * in order to compare it with next position and avoid walls
		 *
		 */
		xyLoc last;
		last.x = next.x;
		last.y = next.y;

		// Calculate distance in each axis
		int d_x = abs(next.x - end.x);
		int d_y = abs(next.y - end.y);

		// Diagonal is preferred than cardinal
		// If diagonal is possible, move diagonal
		if (d_x > 0 && d_y > 0)
		{
			next.x += factor_x;
			next.y += factor_y;
		}
		else if (d_x > 0)
			next.x += factor_x;
		else
			next.y += factor_y;

		// If Next is not a valid position or is diagonal and cardinal are not valid
		if (!map[next.y*width+next.x] || (!area.contains(next) && (abs(next.x - last.x) && abs(next.y - last.y))) || (abs(next.x - last.x) && abs(next.y - last.y)))
		{
			if (!map[last.y*width+next.x])
			{
				// If vertical is not a valid position, step horizontal
				next.x = last.x;
				next.y = last.y + factor_y;
				d_distance++;
				if (!area.contains(next))
					return false;
			}
			else if (!map[next.y*width+last.x])
			{
				// If horizontal is not a valid position, step vertical
				next.y = last.y;
				next.x = last.x + factor_x;
				d_distance++;
				if (!area.contains(next))
					return false;
			}
			else
			{
				// Both cardinal are free, decide which one (area must contain next point)
				// Check horizontal
				if (!area.contains(next))
				{
					next.x = last.x;
					next.y = last.y + factor_y;
					if (!area.contains(next))
					{
						next.y = last.y;
						next.x = last.x + factor_x;
						// If both cardinal directions are invalid, no path
						if (!area.contains(next))
							return false;
					}
					d_distance++;
				}
				else	// Else, it's a diagonal move, add sqrt(2) to real_distance
					d_distance += 1.4142;
			}
		}
		else
			d_distance++;
		path.push_back(next);
	}

	// Round up real distance
	r_distance = (ceil(d_distance) - d_distance > 0.5 ? floor(d_distance) : ceil(d_distance));

	// Path is finished, return it
	return true;
}


bool sort_area_by_left (map_area f, map_area k) { return f.last_left < k.last_left; }
bool sort_area_by_id (map_area f, map_area k) { return f.id < k.id; }

/*
 * Preprocessing Map
 *
 * Extracting Manhattan-cohesion areas.
 * Extracting strategic positions for nodes in order to create the graph.
 *
 */
void PreprocessMap(std::vector<bool> &bits, int w, int h, const char *filename)
{
	// Initialize size
	width = w;
	height = h;
	std::vector<node> map_nodes;

	// Here we preprocess the map
	// We form Manhattan-cohesion areas, where any two points inside an area can be connected with a straight line
	// Straight line is when we can move from one point to other and at each step, we decrease the Manhattan-distance

	// This is the front of the search, the open areas to be expanded
	std::vector<map_area> front;
	front.clear();
	printf("Map Preprocessing...\n");
	// Counter for areas to be created
	int area_counter = 0;
	// Counter for nodes to be created
	int node_counter = 0;
	for (int y=0; y<height; y++)
	{
		// For each row, get row slices of free areas
		std::vector<row_slice> current_slices;
		bool slice_flag = false;
		row_slice current(y);
		for (int x=0; x<width; x++)
		{
			// Parse each line and search for free areas

			//_____ If we are in a slice and next is wall or the above pattern has changed, break slice
			if ((!bits[y*width+x] || (y > 0 && bits[(y-1)*width+x] != bits[(y-1)*width+current.first])) && slice_flag)
			{
				int idd = current_slices.size();
				current.ids.clear();
				current.ids.push_back(idd);
				current_slices.push_back(current);
				slice_flag = false;
			}

			if (bits[y*width+x])
			{
				// If it's a valid point
				// check if it's already inside a slice
				if (!slice_flag)
				{
					// If it's the first point of the slice,
					// create a new slice and set first point
					current.set_first(x);
					slice_flag = true;
				}
				// Set last point as the current point
				current.set_last(x);
			}
		}

		// If there is a last slice with no wall in the end
		// Insert it to the list
		if (slice_flag)
		{
			int idd = current_slices.size();
			current.ids.clear();
			current.ids.push_back(idd);
			current_slices.push_back(current);
		}
		std::vector<int> inserted;
		// Check for every area in the front, which slice can be inserted into an area
		for (unsigned f=0; f<front.size(); f++)
		{
			front[f].to_close = true;
			// Initialize all candidate slices to be inserted (connected slices somehow)
			std::vector<row_slice> connected;
			for (unsigned j=0; j<current_slices.size(); j++)
			{
				// If slice is out of range, break loop
				if (current_slices[j].first > front[f].last_right + 1)
					break;

				// A Connected slice may be in 6 positions:
				// - Exactly below and on the left (possibly connected) (take this slice only if area can go left)
				bool below_left = (current_slices[j].last == front[f].last_left - 1 && front[f].can_go_left());
				// - Exactly below and left of the front (connected but not sure if area can go that way)
				bool left_and_below = (current_slices[j].first < front[f].last_left && front[f].can_go_left() && current_slices[j].last > front[f].last_left && current_slices[j].last <= front[f].last_right);
				// - Exactly below (all areas can go downwards)
				bool below = (current_slices[j].first >= front[f].last_left && current_slices[j].last <= front[f].last_right);
				// - Exactly below and on the right (possibly connected) (take this slice only if area can go right)
				bool below_right = (current_slices[j].first == front[f].last_right + 1 && front[f].can_go_right());
				// - Exactly below and right of the front (connected but not sure if area can go that way)
				bool right_and_below = (current_slices[j].first >= front[f].last_left && current_slices[j].first < front[f].last_right && current_slices[j].last > front[f].last_right && front[f].can_go_right());
				// - Exactly below and on the left and on the right of the front (connected but the front must be able to expand on both directions)
				bool right_and_below_and_left = (current_slices[j].first < front[f].last_left && front[f].can_go_left() && current_slices[j].last > front[f].last_right && front[f].can_go_right());

				// If slice in one position of the above and not inserted yet, is connected
                if ((below_left || left_and_below || below || below_right || right_and_below || right_and_below_and_left) && !current_slices[j].inserted)
					connected.push_back(current_slices[j]);
			}

			// If no connected slices found
			// Proceed to the next front
			if (connected.size() == 0)
				continue;


			// After getting the possibly connected slices
			// - Check if two slices can be merged
			// - Decide which slice can be inserted into the front
			//   - Take the slice with the biggest width
			//	 - Insert it only if width > 10% of last width of area



			// In Details:
			// - Check if two slices can be merged
			std::vector<row_slice> connected_merged;
			row_slice merged = connected[0];
			for (unsigned i=0; i<connected.size()-1; i++)
			{
				// If next slice is right next to current, merge
				if ((connected[i+1].first - connected[i].last) == 1)
					merged.merge_slice(connected[i+1]);
				else
				{
					// Else, insert merged slice to connected_merged
					// and set next slice as the new slice to be merged
					connected_merged.push_back(merged);
					merged = connected[i+1];
				}
			}
			// Insert merged slice
			connected_merged.push_back(merged);
			// - Decide which slice can be inserted into the front (actually connected slices)
			std::vector<row_slice> actually_connected;
			for (unsigned i=0; i<connected_merged.size(); i++)
				if ((connected_merged[i].last >= front[f].last_left && connected_merged[i].last <= front[f].last_right) || (connected_merged[i].first >= front[f].last_left && connected_merged[i].first <= front[f].last_right)|| (connected_merged[i].first <= front[f].last_left && connected_merged[i].last >= front[f].last_right))
				{
					connected_merged[i].area_connected = front[f].id;
					actually_connected.push_back(connected_merged[i]);
				}
			connected_merged.clear();
			//   - Take the slice with the biggest width
			if (actually_connected.size() == 0)
			     continue;
			int max_width = actually_connected[0].size;
			int indx = 0;
			for (unsigned i=1; i<actually_connected.size(); i++)
				if (actually_connected[i].size > max_width)
				{
					max_width = actually_connected[i].size;
					indx = i;
				}
			//	 - Insert it only if width > 10% of last width of area (It prevents from expanding areas with no reason)
			double perc = ((double)max_width / front[f].last_width);
			if ( perc > 0.1)
			{
				// Set wall guide of the front on both sides
				if (front[f].last_left < actually_connected[indx].first)
					front[f].close_left = true;
				else if (front[f].last_left > actually_connected[indx].first)
					front[f].open_left = true;

				if (front[f].last_right < actually_connected[indx].last)
					front[f].open_right = true;
				else if (front[f].last_right > actually_connected[indx].last)
					front[f].close_right = true;

				// Creating nodes before inserting slice to area
				if (f > 0 && ((front[f-1].to_close && front[f-1].last_right >= actually_connected[indx].first) || (!front[f-1].to_close && front[f-1].blast_right >= actually_connected[indx].first)))
				{
					// If it's after the first front, check for connectivity with the previous front in order to create nodes
					int top_start = (front[f-1].last_left < front[f-1].blast_left && front[f-1].blast_left > -1 ? front[f-1].blast_left : front[f-1].last_left);
					top_start = (actually_connected[indx].first < top_start ? top_start : actually_connected[indx].first);
					int top_end = (front[f-1].last_right > front[f-1].blast_right && front[f-1].blast_right > -1 ? front[f-1].blast_right : front[f-1].last_right);
					top_end = (actually_connected[indx].last < top_end ? actually_connected[indx].last : top_end);

					int bottom_start = top_start;
					int bottom_end = top_end;

					// First node on the left
					if (top_end >= top_start)
					{
						node n_left;
						n_left.id = node_counter;

						node_item t1;
						t1.area = front[f-1].id;
						t1.position.y =  actually_connected[indx].row-2;
						t1.position.x = top_start;
						if (!bits[t1.position.y*width + t1.position.x])
							t1.position.y = actually_connected[indx].row-1;
						n_left.items.push_back(t1);
						n_left.areas.push_back(front[f-1].id);

						t1.area = front[f].id;
						t1.position.y =  actually_connected[indx].row;
						t1.position.x = bottom_start;
						n_left.items.push_back(t1);
						n_left.areas.push_back(front[f].id);
						map_nodes.push_back(n_left);
						node_counter++;

						// Last node on the right
						if (top_end > top_start)
						{
							node n_right;
							n_right.id = node_counter;

							node_item t1;
							t1.area = front[f-1].id;
							t1.position.y =  actually_connected[indx].row-2;
							t1.position.x = top_end;
							if (!bits[t1.position.y*width + t1.position.x])
								t1.position.y = actually_connected[indx].row-1;
							n_right.items.push_back(t1);
							n_right.areas.push_back(front[f-1].id);

							t1.area = front[f].id;
							t1.position.y =  actually_connected[indx].row;
							t1.position.x = bottom_end;
							n_right.items.push_back(t1);
							n_right.areas.push_back(front[f].id);
							map_nodes.push_back(n_right);
							node_counter++;

							// Node in the middle of the space between top_start-top_end or bottom_start-bottom_end (choose the smallest distance)
							if (top_end - top_start > 10)
							{
								int middle = (top_start - top_end < top_start - top_end) ? top_start + (top_end - top_start + 1)/2 : bottom_start + (bottom_end - bottom_start + 1)/2;
								node n_middle;
								n_middle.id = node_counter;

								node_item t1;
								t1.area = front[f-1].id;
								t1.position.y =  actually_connected[indx].row-2;
								t1.position.x = middle;
								if (!bits[t1.position.y*width + t1.position.x])
									t1.position.y = actually_connected[indx].row-1;
								n_middle.items.push_back(t1);
								n_middle.areas.push_back(front[f-1].id);

								t1.area = front[f].id;
								t1.position.y =  actually_connected[indx].row;
								t1.position.x = middle;
								n_middle.items.push_back(t1);
								n_middle.areas.push_back(front[f].id);
								map_nodes.push_back(n_middle);
								node_counter++;
							}
						}
					}
				}
				int f1 = 1;
				// If there are still fronts to calculate, check if slice is connected with any of the next fronts in order to create nodes
				if (f < front.size()-1)
					while ((f+f1 < front.size()) && front[f+f1].last_left <= actually_connected[indx].last)
					{
						int top_start = (front[f+f1].last_left < front[f+f1].blast_left && front[f+f1].blast_left > 0 ? front[f+f1].blast_left : front[f+f1].last_left);
						top_start = (actually_connected[indx].first < top_start ? top_start : actually_connected[indx].first);
						int top_end = (front[f+f1].last_right > front[f+f1].blast_right && front[f+f1].blast_right > 0 ? front[f+f1].blast_right : front[f+f1].last_right);
						top_end = (actually_connected[indx].last < top_end ? actually_connected[indx].last : top_end);

						int bottom_start = (front[f+f1].last_left < front[f+f1].blast_left && front[f+f1].blast_left > 0 ? front[f+f1].blast_left : front[f+f1].last_left);
						bottom_start = top_start;
						int bottom_end = (front[f+f1].last_right > front[f+f1].blast_right && front[f+f1].blast_right > 0 ? front[f+f1].blast_right : front[f+f1].last_right);
						bottom_end = top_end;

						// First node on the left
						if (top_end >= top_start)
						{
							node n_left;
							n_left.id = node_counter;

							node_item t1;
							t1.area = front[f+f1].id;
							t1.position.y =  actually_connected[indx].row-2;
							t1.position.x = top_start;
							if (!bits[t1.position.y*width + t1.position.x])
								t1.position.y = actually_connected[indx].row-1;
							n_left.items.push_back(t1);
							n_left.areas.push_back(front[f+f1].id);

							t1.area = front[f].id;
							t1.position.y =  actually_connected[indx].row;
							t1.position.x = bottom_start;
							n_left.items.push_back(t1);
							n_left.areas.push_back(front[f].id);
							map_nodes.push_back(n_left);
							node_counter++;

							if (top_end > top_start)
							{
								// Last node on the right
								node n_right;
								n_right.id = node_counter;

								node_item t1;
								t1.area = front[f+f1].id;
								t1.position.y =  actually_connected[indx].row-2;
								t1.position.x = top_end;
								if (!bits[t1.position.y*width + t1.position.x])
									t1.position.y = actually_connected[indx].row-1;
								n_right.items.push_back(t1);
								n_right.areas.push_back(front[f+f1].id);

								t1.area = front[f].id;
								t1.position.y =  actually_connected[indx].row;
								t1.position.x = bottom_end;
								n_right.items.push_back(t1);
								n_right.areas.push_back(front[f].id);
								map_nodes.push_back(n_right);
								node_counter++;

								// Node in the middle of the space between top_start-top_end or bottom_start-bottom_end (choose the smallest distance)
								if (top_end - top_start > 10)
								{
									int middle = (top_start - top_end < top_start - top_end) ? top_start + (top_end - top_start + 1)/2 : bottom_start + (bottom_end - bottom_start + 1)/2;
									node n_middle;
									n_middle.id = node_counter;

									node_item t1;
									t1.area = front[f+f1].id;
									t1.position.y =  actually_connected[indx].row-1;
									t1.position.x = middle;
									n_middle.items.push_back(t1);
									n_middle.areas.push_back(front[f+f1].id);

									t1.area = front[f].id;
									t1.position.y =  actually_connected[indx].row;
									t1.position.x = middle;
									n_middle.items.push_back(t1);
									n_middle.areas.push_back(front[f].id);
									map_nodes.push_back(n_middle);
									node_counter++;
								}
							}
						}
						f1++;
					}
				// Inserting slice to area
				front[f].addPoints(actually_connected[indx].row, actually_connected[indx].first, actually_connected[indx].last);
				front[f].to_close = false;

				// Store ids of inserted slices
				for (unsigned i=0; i<actually_connected[indx].ids.size(); i++)
				{
					inserted.push_back(actually_connected[indx].ids[i]);
					current_slices[actually_connected[indx].ids[i]].area_inserted = front[f].id;
					current_slices[actually_connected[indx].ids[i]].current_front = f;
				}
			}
			// Set boolean for the slices that already inserted
			for (unsigned j=0; j<inserted.size(); j++)
				current_slices[inserted[j]].inserted = true;
		}
		// For the slices which weren't inserted anywhere, create new areas
		// And insert them into the front
		for (unsigned i=0; i<current_slices.size(); i++)
		{
			if (!current_slices[i].inserted)
			{
				// If slice didn't inserted anywhere, check for nodes and create them
				for (unsigned k=0; k<front.size(); k++)
				{
					if (!(current_slices[i].last < front[k].last_left && current_slices[i].last < front[k].blast_left) && !(current_slices[i].first > front[k].last_right && current_slices[i].first > front[k].blast_right))
					{
						int l_left = (front[k].to_close ? front[k].last_left : front[k].blast_left);
						int l_right = (front[k].to_close ? front[k].last_right : front[k].blast_right);
						int start = ((current_slices[i].first < l_left) ? l_left : current_slices[i].first);
						int end = ((current_slices[i].last > l_right) ? l_right : current_slices[i].last);

						// First node on the left
						if (end < start)
							continue;
						node n_left;
						n_left.id = node_counter;

						node_item t1;
						t1.area = front[k].id;
						t1.position.y = current_slices[i].row-1;
						t1.position.x = start;
						n_left.items.push_back(t1);
						n_left.areas.push_back(front[k].id);

						t1.area = area_counter;
						t1.position.y = current_slices[i].row;
						t1.position.x = start;
						n_left.items.push_back(t1);
						n_left.areas.push_back(area_counter);
						map_nodes.push_back(n_left);
						node_counter++;

						if (end == start)
							continue;
						// First node on the right
						node n_right;
						n_right.id = node_counter;

						t1.area = front[k].id;
						t1.position.y = current_slices[i].row-1;
						t1.position.x = end;
						n_right.items.push_back(t1);
						n_right.areas.push_back(front[k].id);

						t1.area = area_counter;
						t1.position.y = current_slices[i].row;
						t1.position.x = end;
						n_right.items.push_back(t1);
						n_right.areas.push_back(area_counter);
						map_nodes.push_back(n_right);
						node_counter++;


						// Node in the middle of the space between top_start-top_end or bottom_start-bottom_end (choose the smallest distance)
						if (end - start < 10)
							continue;
						int middle = start + (end - start + 1)/2;
						node n_middle;
						n_middle.id = node_counter;

						t1.area = front[k].id;
						t1.position.y =  current_slices[i].row-1;
						t1.position.x = middle;
						n_middle.items.push_back(t1);
						n_middle.areas.push_back(front[k].id);

						t1.area = area_counter;
						t1.position.y =  current_slices[i].row;
						t1.position.x = middle;
						n_middle.items.push_back(t1);
						n_middle.areas.push_back(area_counter);
						map_nodes.push_back(n_middle);
						node_counter++;
					}
				}

				// Create a new area
				map_area temp(area_counter);
				temp.addPoints(current_slices[i].row, current_slices[i].first, current_slices[i].last);
				front.push_back(temp);
				current_slices[i].area_inserted = area_counter;
				area_counter++;
			}

			// If two slices are connected and inserted to different areas, create node
			if (i > 0 && current_slices[i-1].area_inserted != current_slices[i].area_inserted && current_slices[i-1].last == current_slices[i].first-1 && current_slices[i].row > 0)// && (bits[(current_slices[i].row-1)*width + current_slices[i-1].last] || front[current_slices[i].current_front].can_go_left()))
			{
				node t;
				t.id = node_counter;

				node_item t1;
				t1.area = current_slices[i-1].area_inserted;
				t1.position.y = current_slices[i].row;
				t1.position.x = current_slices[i-1].last;
				t.items.push_back(t1);
				t.areas.push_back(current_slices[i-1].area_inserted);

				t1.area = current_slices[i].area_inserted;
				t1.position.y = current_slices[i].row;
				t1.position.x = current_slices[i].first;
				t.items.push_back(t1);
				t.areas.push_back(current_slices[i].area_inserted);
				map_nodes.push_back(t);
				node_counter++;
			}
		}



		// After expanding fronts or creating new
		// Remove the fronts that didn't append to any area and insert them to the final areas
		std::vector<map_area> temp;
		int f_to_close = 0;
		int f_not_to_close = 0;
		for (unsigned i=0; i<front.size(); i++)
		{
			if (!front[i].to_close)
			{
				temp.push_back(front[i]);
				f_not_to_close++;
			}
			else
			{
				final_areas.push_back(front[i]);
				f_to_close++;
			}
		}
		front.clear();
		front = temp;

		// Reset all areas of the front to close on next loop
		for (unsigned i=0; i<front.size(); i++)
			front[i].to_close = true;

		// Sort fronts by last_left
		sort(front.begin(), front.end(), sort_area_by_left);
	}
	// Insert the last fronts
	for (unsigned i=0; i<front.size(); i++)
		final_areas.push_back(front[i]);

	// Sort final areas with id
	sort(final_areas.begin(), final_areas.end(), sort_area_by_id);
	printf("Total areas created: %d\n", final_areas.size());
	printf("Total nodes created: %d\n", map_nodes.size());


	// Creating real nodes and successors (Creating graph)
	printf("Creating graph...\n");

	sort(map_nodes.begin(), map_nodes.end(), sort_nodes_by_id);
	// Asserting nodes to areas
	for (unsigned i=0; i<map_nodes.size(); i++)
	{
		int area_id1 = map_nodes[i].areas[0];
		int area_id2 = map_nodes[i].areas[1];
		int node_id = map_nodes[i].id;
		final_areas[area_id1].nodes.push_back(node_id);
		final_areas[area_id2].nodes.push_back(node_id);
	}

	// Creating children and completing graph
	final_nodes.clear();
	for (unsigned i=0; i<map_nodes.size(); i++)
	{
		/*
		 * For every node, search the nodes of the connected areas.
		 * For every other node, create line
		 *
		 */
		node final = map_nodes[i];
		map_area area1 = final_areas[final.areas[0]];
		map_area area2 = final_areas[final.areas[1]];

		// Setting children from first area
		unsigned node_count1 = area1.nodes.size();
		for (unsigned j=0; j<node_count1; j++)
		{
			node temp = map_nodes[area1.nodes[j]];
			bool match = ((final.areas[0] == temp.areas[0] && final.areas[1] != temp.areas[1]) || (final.areas[0] == temp.areas[1] && final.areas[1] != temp.areas[0]) || (final.areas[1] == temp.areas[0] && final.areas[0] != temp.areas[1]) || (final.areas[1] == temp.areas[1] && final.areas[0] != temp.areas[0]));
			if (match)
			{
				int area_id;
				xyLoc start, end;
				if (final.areas[0] == temp.areas[0] || final.areas[0] == temp.areas[1])
				{
					area_id = final.areas[0];
					start = final.items[0].position;
					end = (final.areas[0] == temp.areas[0] ? temp.items[0].position : temp.items[1].position);
				}
				else
				{
					area_id = final.areas[1];
					start = final.items[1].position;
					end = (final.areas[1] == temp.areas[0] ? temp.items[0].position : temp.items[1].position);
				}
				std::vector<xyLoc> mhPath;
				get_str_path(bits, start, end, final_areas[area_id], mhPath, real_distance);
				temp.distance = real_distance;
				final.children.push_back(temp);
			}
		}

		// Setting children from second area
		unsigned node_count2 = area2.nodes.size();
		for (unsigned j=0; j<node_count2; j++)
		{
			node temp = map_nodes[area2.nodes[j]];
			bool match = ((final.areas[0] == temp.areas[0] && final.areas[1] != temp.areas[1]) || (final.areas[0] == temp.areas[1] && final.areas[1] != temp.areas[0]) || (final.areas[1] == temp.areas[0] && final.areas[0] != temp.areas[1]) || (final.areas[1] == temp.areas[1] && final.areas[0] != temp.areas[0]));
			if (match)
			{
				int area_id;
				xyLoc start, end;
				if (final.areas[0] == temp.areas[0] || final.areas[0] == temp.areas[1])
				{
					area_id = final.areas[0];
					start = final.items[0].position;
					end = (final.areas[0] == temp.areas[0] ? temp.items[0].position : temp.items[1].position);
				}
				else
				{
					area_id = final.areas[1];
					start = final.items[1].position;
					end = (final.areas[1] == temp.areas[0] ? temp.items[0].position : temp.items[1].position);
				}
				std::vector<xyLoc> mhPath;
				get_str_path(bits, start, end, final_areas[area_id], mhPath, real_distance);
				temp.distance = real_distance;
				final.children.push_back(temp);
			}
		}
		if (final.children.size() > 0)
			final_nodes.push_back(final);
	}
	sort(final_nodes.begin(), final_nodes.end(), sort_nodes_by_id);
	// Resize final_nodes in order to insert starting and goal nodes
	printf("Graph successfully created!\n");
	export_map(final_areas, final_nodes, filename);
	final_nodes.resize(final_nodes.size() + 2);
	printf("Pre-processing OK!\n");
}

/*
 * Exporting map to a text-file
 *
 */
void export_map(std::vector<map_area> areas, std::vector<node> nodes, const char* filename)
{
	// Write to file
	ofstream myfile;
	myfile.open (filename);
	myfile << ":areas "<< areas.size() << std::endl;
	for (unsigned i = 0; i < areas.size(); i++)
	{
		myfile << ":area " << areas[i].id << "|" << areas[i].start_row << "," << areas[i].row_count << std::endl;
		for (unsigned j=0; j<areas[i].start_points.size(); j++)
			myfile << areas[i].start_points[j] << "-" << areas[i].end_points[j] << "|";
		myfile << std::endl;
	}
	myfile << ":nodes "<< nodes.size() <<"\n";
	for (unsigned i = 0; i < nodes.size(); i++)
	{
		myfile << ":node " << nodes[i].id << "=" << nodes[i].items[0].area << ":" << nodes[i].items[0].position.x << "," << nodes[i].items[0].position.y << "|" << nodes[i].items[1].area << ":" << nodes[i].items[1].position.x << "," << nodes[i].items[1].position.y << std::endl;
		myfile << ":children " << nodes[i].children.size() << std::endl;
		for (unsigned j=0; j<nodes[i].children.size(); j++)
			myfile << nodes[i].children[j].id << ":" << nodes[i].children[j].distance << "|";
		myfile << std::endl;
	}
	myfile.close();
}


/*
 * Graph Node
 *
 */
node::node()
{
	this->total_distance = 0;
	this->parent = -1;
	this->visited = false;
	this->temp = false;
	this->depth = 0;
}

/*
 * Map Area
 *
 */
map_area::map_area(int id)
{
	this->id = id;
	this->to_close = false;
	this->last_width = 1;
	this->open_left = false;
	this->open_right = false;
	this->close_left = false;
	this->close_right = false;
	this->last_left = 0;
	this->last_right = 0;
	this->blast_left = -1;
	this->blast_right = -1;
	this->start_row = -1;
	this->row_count = 0;
}

bool map_area::can_go_left()
{
	return !this->close_left;
}
bool map_area::can_go_right()
{
	return !this->close_right;
}
void map_area::addPoints(int row, int start, int end)
{
	if (this->start_row == -1)
		this->start_row = row;
	this->row_count++;
	this->start_points.push_back(start);
	this->end_points.push_back(end);

	this->blast_left = this->last_left;
	this->blast_right = this->last_right;
	this->last_left = start;
	this->last_right = end;
	this->last_width = end - start + 1;
}
bool map_area::contains(xyLoc point)
{
	if (point.y < this->start_row || point.y >= (this->start_row + this->row_count))
		return false;
	int indx = point.y - this->start_row;
	return (point.x >= this->start_points[indx] && point.x <= this->end_points[indx]);
}



/*
 * Map Slice
 *
 */
row_slice::row_slice(int y)
{
	this->row = y;
	this->first = 0;
	this->last = 0;

	this->inserted = false;
	this->area_connected = -1;
}
void row_slice::set_first(int x)
{
	this->first = x;
}
void row_slice::set_last(int x)
{
	this->last = x;
	this->size = this->last - this->first + 1;
}

void row_slice::append_points(int end)
{
	this->last = end;
	this->size = this->last - this->first + 1;

}
void row_slice::merge_slice(row_slice to_merge)
{
	for (unsigned i=0; i<to_merge.ids.size(); i++)
		this->ids.push_back(to_merge.ids[i]);
	this->append_points(to_merge.last);
}


/*
 * Map Visualizer
 *
 */
void write_map(std::vector<bool> map, int width, int height, xyLoc start, xyLoc end, std::vector<xyLoc> path)
{
	std::vector<string> tempMap;
	tempMap.resize(height*width);

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			tempMap[y*width+x] = (map[y*width+x] ? "." : "@");
	obscure_area(tempMap, width, height, start);
	obscure_area(tempMap, width, height, end);
	write_path(tempMap, width, height, path);

	write_file("visual.txt", tempMap, width, height);

}
void write_map(std::vector<bool> map, int width, int height, std::vector<map_area> areas, std::vector<node> nodes)
{
	printf("Writing map to file...\n");
	std::vector<string> tempMap;
		tempMap.resize(height*width);

		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				tempMap[y*width+x] = (map[y*width+x] ? "." : "@");

		for (unsigned i=0; i< areas.size(); i++)
		{
	        string c = "(";
	        switch (i%11)
	        {
	            case 0:
	                c = "A";
	                break;
	            case 1:
	                c = "B";
	                break;
	            case 2:
	                c = "C";
	                break;
	            case 3:
	                c = "D";
	                break;
	            case 4:
	                c = "E";
	                break;
	            case 5:
	                c = "F";
	                break;
	            case 6:
	                c = "G";
	                break;
	            case 7:
	                c = "H";
	                break;
	            case 8:
	                c = "I";
	                break;
	            case 9:
	                c = "J";
	                break;
	            case 10:
	                c = "K";
	                break;
	            case 11:
	                c = "L";
	                break;
	        }
	        for (int j=0; j<areas[i].row_count; j++)
				for (int k=areas[i].start_points[j]; k<=areas[i].end_points[j]; k++)
				{
					int row = areas[i].start_row + j;
					tempMap[row*width+k] = c;
				}
		}
		for (unsigned i=0; i<nodes.size(); i++)
		{
			tempMap[nodes[i].items[0].position.y*width+nodes[i].items[0].position.x] = "-";
			tempMap[nodes[i].items[1].position.y*width+nodes[i].items[1].position.x] = "-";
		}
		write_file("visual.txt", tempMap, width, height);
}
void write_map_path(std::vector<string> &tempMap, int width, int height, xyLoc sp, xyLoc gp, std::vector<xyLoc> path)
{
	obscure_area(tempMap, width, height, sp);
	obscure_area(tempMap, width, height, gp);

	for (unsigned i=0; i<path.size(); i++)
		tempMap[path[i].y*width+path[i].x] = "*";
}

void write_file(const char *fname, std::vector<string> tempMap, int width, int height)
{
	// Write to file
	ofstream myfile;
	myfile.open (fname);

	myfile << width<<"\n";
	myfile << height<<"\n";
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
			myfile << (tempMap[y*width+x]);

		myfile << std::endl;
	}

	myfile.close();
}

void obscure_area(std::vector<string> &tempMap,int width,int height, xyLoc point)
{
	xyLoc p = point;
	tempMap[p.y*width+p.x] = "#";

	// top left
	p = point;
	p.y--;
	p.x--;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";

	// top
	p = point;
	p.y--;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";

	// top right
	p = point;
	p.y--;
	p.x++;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";

	// left
	p = point;
	p.x--;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";

	// right
	p = point;
	p.x++;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";

	// bottom left
	p = point;
	p.y++;
	p.x--;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";

	// bottom
	p = point;
	p.y++;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";

	// bottom right
	p = point;
	p.y++;
	p.x++;
	p.x = (p.x < 0 ? 0 : (p.x > width ? width : p.x));
	p.y = (p.y < 0 ? 0 : (p.y > height ? height : p.y));
	tempMap[p.y*width+p.x] = "-";
}
void write_path(std::vector<string> &tempMap, int width, int height, std::vector<xyLoc> path)
{
	printf("size: %d\n",path.size());
	for (unsigned i=0; i<path.size(); i++)
		tempMap[path[i].y*width+path[i].x] = "*";
}
