#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "pdb.h"
#include "hash.h"
#include "Entry.h"
#include "search.h"
#include "map.h"

using namespace std;

pdb_t *pdb = 0;
int g_width;

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{
	g_width = width;
	return; // no writing
}

void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename)
{
	g_width = width;
	map_t *map = new_map( width, height );
	if (pdb == 0)
		pdb = build_block_pdb();
	
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			//cell_entry_t i = map_cell(map, x, y);
			xyLoc a;
			a.x = x;
			a.y = y;
			if (bits[GetIndex(a)])
				map_cell( map, x, y ).is_obstacle = 0;
			else 
				map_cell( map, x, y ).is_obstacle = 1;
		}
		for( int y = height; y < map->height; ++y ) {
		  map_cell( map, x, y ).is_obstacle = 1;
		}
	}
	for( int x = width; x < map->width; ++x ) {
	  for( int y = 0; y < map->height; ++y ) {
	    map_cell( map, x, y ).is_obstacle = 1;
	  }
	}
	recompute_cells( map );

	return map;
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	path.resize(0);
	map_t *map = (map_t*)data;
	int start, goal;
	start = map_cell_index(map, s.x, s.y);
	goal = map_cell_index(map, g.x, g.y);
	int dist = block_astar( map, pdb, start, goal );
	//printf("(dist %d)", dist);
	if( dist < unreachable ) {
        CellIdx path1[ dist + 1 ];
        block_astar_path( map, pdb, start, goal, path1 );
		for (int x = dist; x >= 0; --x)
		{
			xyLoc res;
			res.x = map_index_x(map, path1[x]);
			res.y = map_index_y(map, path1[x]);
			path.push_back(res);
		}
	}
	return true;
}

int GetIndex(xyLoc s)
{
	return s.y*g_width+s.x;
}

const char *GetName()
{
	return "block-astar";
}

