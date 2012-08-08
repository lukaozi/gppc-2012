#ifdef __cplusplus
extern "C" {
#endif


#ifndef _PDB_H
#define _PDB_H

#include "map.h"


#define pdb_unreachable UINT8_MAX
#define MAX_PATH_POINTS 11


typedef struct {
  uint8_t distance;
  /* !!NOTE!!  THE PATH IS IN REVERSE ORDER: OUT TO IN */
  int8_t path_points[ MAX_PATH_POINTS+1 ]; /* terminated by -1 */
} pdb_distance_t;


typedef struct {
  /* distances[ ( block_bitstring * BLOCK_SIZE + entrance )
		* PERIMETER_SIZE + perimeter_index ]
		gives the shortest distance from the entrance cell
		(along the top side) to all possible perimeter cells */
  pdb_distance_t *distances;
} pdb_t;


pdb_t *build_block_pdb();

/* free all memory associated with pdb */
void destroy_pdb( pdb_t *pdb );

/* update distances for child block, given a move from current block
   in the given direction.  returns the minimum f value of any
   updated cell, or unreachable if no distance was updated */
distance_t update_distances( const map_t *map, const pdb_t *pdb,
			     const CellIdx goal_cell, const int8_t dir,
			     const BlockIdx cur_block,
			     const distance_t *cur_dists,
			     const BlockIdx child_block,
			     distance_t *child_dists, BlockIdx *parents );
#endif

#ifdef __cplusplus
}
#endif
