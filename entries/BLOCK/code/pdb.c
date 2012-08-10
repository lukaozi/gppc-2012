#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "pdb.h"


/* this function ignores block->width and block->height 
   but block->size must be at least BLOCK_SIZE*BLOCK_SIZE */
static void unpack_block_bits( block_bitstring_t block_bits, map_t *block )
{
  block_bitstring_t i;
  CellIdx y, x;

  i = 1;
  for( y = 0; y < BLOCK_SIZE; ++y ) {

    for( x = 0; x < BLOCK_SIZE; i <<= 1, ++x ) {

      if( block_bits & i ) {

	block->cells[ y * BLOCK_SIZE + x ].is_obstacle = 1;
      } else {

	block->cells[ y * BLOCK_SIZE + x ].is_obstacle = 0;
      }
    }
  }

  recompute_cells( block );
}

pdb_t *build_block_pdb()
{
  block_bitstring_t pdb_size, block_bits;
  CellIdx i, j, k, t;

  pdb_t *pdb;

  map_t *block;
  distance_t *block_distances;
  CellIdx *block_parents;

  /* ------------------------------------------------------------ */
  /* create the PDB */

  pdb = (pdb_t*)malloc( sizeof( *pdb ) );
  assert( pdb != NULL );
  pdb->distances = malloc( sizeof( pdb->distances[ 0 ] )
			   * ( 1 << ( BLOCK_SIZE * BLOCK_SIZE ) )
			   * BLOCK_SIZE * PERIMETER_SIZE );
  assert( pdb->distances != NULL );

  /* ------------------------------------------------------------ */
  /* create the map for the block */

  block = new_map( BLOCK_SIZE, BLOCK_SIZE );
  assert( block != NULL );

  /* ------------------------------------------------------------ */
  /* create space for the distances and parents */

  block_distances = malloc( sizeof( *block_distances )
			    * BLOCK_SIZE * BLOCK_SIZE );
  assert( block_distances != NULL );
  block_parents = malloc( sizeof( *block_parents )
			  * BLOCK_SIZE * BLOCK_SIZE );
  assert( block_parents != NULL );

  /* ------------------------------------------------------------ */
  /* try all possible blocks */

  pdb_size = 1 << block->size;
  for( block_bits = 0; block_bits < pdb_size; ++block_bits ) {

    unpack_block_bits( block_bits, block );

    for( i = 0; i < BLOCK_SIZE; ++i ) {

      get_distances( block, i, 1, block_distances, block_parents );

      /* save the distances and paths for entrance i */
      for( j = 0; j < PERIMETER_SIZE; ++j ) {

	if( block_distances[ block->perimeter_indices[ j ] ] != unreachable ) {

	  /* store distance */
	  pdb->distances[ ( block_bits * BLOCK_SIZE + i )
			  * PERIMETER_SIZE + j ].distance
	    = block_distances[ block->perimeter_indices[ j ] ];

	  /* store path */
	  k = 0;
	  t = block->perimeter_indices[ j ];
	  while( block_parents[ t ] >= 0 ) {
	    t = block_parents[ t ];
	    pdb->distances[ ( block_bits * BLOCK_SIZE + i )
			    * PERIMETER_SIZE + j ].path_points[ k ] = t;
	    ++k;
	    if( k >= MAX_PATH_POINTS ) {
	      fprintf( stderr, "%d\n", k );
	      assert( k < MAX_PATH_POINTS );
	    }
	  }
	  pdb->distances[ ( block_bits * BLOCK_SIZE + i )
			  * PERIMETER_SIZE + j ].path_points[ k ] = -1;
	} else {
	  /* perimeter index j is unreachable from entrance i */

	  pdb->distances[ ( block_bits * BLOCK_SIZE + i )
			  * PERIMETER_SIZE + j ].distance
	    = pdb_unreachable;
	  pdb->distances[ ( block_bits * BLOCK_SIZE + i )
			  * PERIMETER_SIZE + j ].path_points[ 0 ] = -1;
	}
      }
    }
  }

  /* ------------------------------------------------------------ */
  /* clean up */

  free( block_parents );
  free( block_distances );
  destroy_map( block );

  /* ------------------------------------------------------------ */

  return pdb;
}

void destroy_pdb( pdb_t *pdb )
{
  if( pdb ) {

    if( pdb->distances ) {
      free( pdb->distances );
    }

    free( pdb );
  }
}


distance_t update_distances( const map_t *map, const pdb_t *pdb,
			     const CellIdx goal_cell, const int8_t dir,
			     const BlockIdx cur_block,
			     const distance_t *cur_dists,
			     const BlockIdx child_block,
			     distance_t *child_dists, BlockIdx *parents )
{
  CellIdx entrance, *edge_out, *perimeter_in, idx, cell;
  BlockIdx parent;
  distance_t min_f, entrance_g, dist_f;
  block_bitstring_t block_bitstring;
  pdb_distance_t *pdb_dists;
  int8_t i;

  edge_out = &map->edge_out[ dir * BLOCK_SIZE ];
  perimeter_in = &map->perimeter_in[ dir * PERIMETER_SIZE ];
  min_f = unreachable;
  block_bitstring = get_map_block_bits( map, child_block, dir );

  for( entrance = 0; entrance < BLOCK_SIZE; ++entrance ) {

    entrance_g = cur_dists[ edge_out[ entrance ] ];
    if( entrance_g == unreachable ) {
      /* don't update from unreachable cells */

      continue;
    }

    if( child_dists[ perimeter_in[ entrance ] ] <= entrance_g + 1 ) {
      /* the cell adjacent to entry is already as close to the start
	 as going through entrance, so no values will be  updated */

      continue;
    }

    parent = dir * BLOCK_SIZE + entrance;
    pdb_dists = &pdb->distances[ ( block_bitstring * BLOCK_SIZE + entrance )
				 * PERIMETER_SIZE ];

    /* update each perimeter distance */
    for( i = 0; i < PERIMETER_SIZE; ++i ) {

      idx = perimeter_in[ i ];

      if( pdb_dists[ i ].distance != pdb_unreachable
	  && pdb_dists[ i ].distance + entrance_g < child_dists[ idx ] ) {

	child_dists[ idx ] = pdb_dists[ i ].distance + entrance_g;
	parents[ idx ] = parent;

	cell = map->perimeter_offsets[ idx ]
	  + block_topleft_idx( map, child_block );
	dist_f = child_dists[ idx ]
	  + mh_dist_cell( map, cell, goal_cell );
	if( dist_f < min_f ) {

	  min_f = dist_f;
	}
      }
    }
  }

  return min_f;
}
