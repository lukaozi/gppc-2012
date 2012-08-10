#ifdef __cplusplus
extern "C" {
#endif

#include "map.h"
#include "pdb.h"
#include "hash.h"

void astar_path( const map_t *map,
				 const CellIdx start,
				 const CellIdx goal,
				 CellIdx *path );

void block_astar_path( const map_t *map,
					   const pdb_t *pdb,
					   const CellIdx start_cell,
					   const CellIdx goal_cell,
					   CellIdx *path );

distance_t block_astar( map_t *map, const pdb_t *pdb,
						const CellIdx start_cell, const CellIdx goal_cell );

#ifdef __cplusplus
}
#endif
