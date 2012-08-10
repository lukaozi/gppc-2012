#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "map.h"
#include "pdb.h"
#include "hash.h"


#define VERBOSE
#define COUNT_HEAP_OPS


#define IDASTAR_SEARCH 0
#define ASTAR_SEARCH 1
#define BLOCK_ASTAR_SEARCH 2

int64_t g_expanded;
int64_t g_heap_ops;


static int8_t cell_idastar_r( const map_t *map,
			      const CellIdx cur, const CellIdx goal,
			      const distance_t rem_distance,
			      const int8_t skip_move,
			      hash_map_t *ttable, int8_t *proven_unreachable )
{
  CellIdx child;
  int8_t i;
  distance_t *d;

  /* we already checked for the goal case */

  /* check if we have sufficient distance to expand current cell */
  d = hash_map_get_entry( ttable, cur );
  if( d == NULL ) {
    /* child is new to us! */

    if( rem_distance == 0 ) {
      /* no remaining distance, can't expand child  */

      *proven_unreachable = 0;
      return 0;
    }

    if( mh_dist_cell( map, cur, goal ) > rem_distance ) {
      /* heuristic says we're farther than remaining distance - failure! */

      *proven_unreachable = 0;
      return 0;
    }

    hash_map_add_entry( ttable, cur, &rem_distance );
  } else {

    if( *d >= rem_distance ) {
      /* we've already seached child with sufficient remaining depth */

      return 0;
    }

    /* we're searching to a new larger remaining depth */
    *d = rem_distance;

    if( mh_dist_cell( map, cur, goal ) > rem_distance ) {
      /* heuristic says we're farther than remaining distance - failure! */

      return 0;
    }
  }


  /* cell is being expanded */
  ++g_expanded;

  /* try all moves */
  for( i = 0; i < 4; ++i ) {
    if( i == skip_move ) { continue; }

    child = make_move( map, cur, i );
    if( child >= 0 ) {
      /* child is a valid move */

      if( child == goal ) {
	/* we reached the goal - success! */

	return 1;
      }

      if( cell_idastar_r( map, child, goal, rem_distance - 1,
			  i ^ 2, ttable, proven_unreachable ) ) {
	/* found a path - success! */

	return 1;
      }
    }
  }

  /* didn't find a path - failure! */
  return 0;
}

/* returns distance at which a path was found,
   or unreachable if goal can not be reached */
distance_t cell_idastar( const map_t *map,
			 const CellIdx start, const CellIdx goal )
{
  static hash_map_t *ttable;
  distance_t distance;
  int8_t proven_unreachable;

  if( ttable != NULL ) {

    clear_hash_map( ttable );
  } else {

    ttable = init_hash_map( sizeof( distance_t ), 0.75 );
  }

  if( start == goal ) {

    return 0;
  }

  
  for( distance = mh_dist_cell( map, start, goal ); 1; distance += 2 ) {

    proven_unreachable = 1;
    if( cell_idastar_r( map, start, goal, distance, -1,
			ttable, &proven_unreachable ) ) {

      break;
    }

    if( proven_unreachable ) {

      return unreachable;
    }
  }

  return distance;
}


typedef struct {
  distance_t dist_f;
  CellIdx cell;
} cell_heap_entry_t;

static inline
void add_to_cell_heap( cell_heap_entry_t *heap, CellIdx *in_heap,
		       map_t *map, const cell_heap_entry_t *entry )
{
  CellIdx idx;

  idx = *in_heap;
  while( idx && entry->dist_f < heap[ ( idx - 1 ) / 2 ].dist_f ) {

    heap[ idx ] = heap[ ( idx - 1 ) / 2 ];
    map->cells[ heap[ idx ].cell ].heap_pos = idx;
    idx = ( idx - 1 ) / 2;
#ifdef COUNT_HEAP_OPS
    ++g_heap_ops;
#endif
  }

  heap[ idx ] = *entry;
  map->cells[ entry->cell ].heap_pos = idx;
  ++( *in_heap );
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif
}

/* entry's value has decreased, move it up */
static inline
void cell_heapify_up( cell_heap_entry_t *heap, map_t *map, CellIdx idx )
{
  cell_heap_entry_t t;

  if( idx == 0 ) {
    return;
  }

  t = heap[ idx ];
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif

  while( t.dist_f < heap[ ( idx - 1 ) / 2 ].dist_f ) {

    heap[ idx ] = heap[ ( idx - 1 ) / 2 ];
    map->cells[ heap[ idx ].cell ].heap_pos = idx;
    idx = ( idx - 1 ) / 2;
#ifdef COUNT_HEAP_OPS
    ++g_heap_ops;
#endif

    if( idx == 0 ) {
      break;
    }
  }

  heap[ idx ] = t;
  map->cells[ heap[ idx ].cell ].heap_pos = idx;
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif
}

static inline
void get_from_cell_heap( cell_heap_entry_t *heap, CellIdx *in_heap,
			 map_t *map, cell_heap_entry_t *entry )
{
  CellIdx idx;

  *entry = heap[ 0 ];
  map->cells[ entry->cell ].heap_pos = -1;
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif

  --( *in_heap );
  if( *in_heap ) {

    idx = 0;
    while( idx * 2 + 1 < *in_heap ) {

      if( idx * 2 + 2 < *in_heap
	  && heap[ idx * 2 + 1 ].dist_f > heap[ idx * 2 + 2 ].dist_f
	  && heap[ idx * 2 + 2 ].dist_f < heap[ *in_heap ].dist_f ) {

	heap[ idx ] = heap[ idx * 2 + 2 ];
	map->cells[ heap[ idx ].cell ].heap_pos = idx;
	idx = idx * 2 + 2;
#ifdef COUNT_HEAP_OPS
	++g_heap_ops;
#endif
      } else if( heap[ idx * 2 + 1 ].dist_f < heap[ *in_heap ].dist_f ) {

	heap[ idx ] = heap[ idx * 2 + 1 ];
	map->cells[ heap[ idx ].cell ].heap_pos = idx;
	idx = idx * 2 + 1;
#ifdef COUNT_HEAP_OPS
	++g_heap_ops;
#endif
      } else {

	break;
      }
    }

    heap[ idx ] = heap[ *in_heap ];
    map->cells[ heap[ idx ].cell ].heap_pos = idx;
  }
}

/* get path, from goal to start, terminated with -1
   must have sufficient space in path[] for the complete path
   must not be called with unreachable start/goal pairs! */
void astar_path( const map_t *map,
				 const CellIdx start,
				 const CellIdx goal,
				 CellIdx *path )
{
  CellIdx path_len, cell;

  path_len = 0;

  cell = goal;
  while( 1 ) {

    path[ path_len ] = cell;
    ++path_len;

    if( map->cells[ cell ].parent < 0 ) {
      break;
    }
    cell = map->cells[ cell ].parent;
  }

  path[ path_len ] = -1;
}

distance_t cell_astar( map_t *map,
		       const CellIdx start, const CellIdx goal )
{
  static hash_set_t *hash = NULL;
  CellIdx in_heap;
  cell_heap_entry_t *heap, cur_entry, child_entry;
  distance_t child_dist_g;
  int8_t i;

  if( start == goal ) {

    return 0;
  }

  /* make sure we have distance hash table

     it is static because we want to have the hash table grow as few
     times as possible while searching, but we want it to be as small
     as is reasonable because each entry must be initialised, so we
     re-use the hash table and size from older searches */
  if( hash != NULL ) {

    clear_hash_set( hash );
  } else {

    hash = init_hash_set( 0.75 );
  }

  /* create heap */
  heap = malloc( sizeof( *heap ) * map->size );
  in_heap = 0;

  /* set up as though we just pulled start from heap */
  cur_entry.cell = start;
  map->cells[ cur_entry.cell ].dist_g = 0;
  map->cells[ cur_entry.cell ].heap_pos = -1;
  map->cells[ cur_entry.cell ].parent = -1;
  hash_set_add_key( hash, cur_entry.cell );

  /* search! */
  while( cur_entry.cell != goal ) {

    ++g_expanded;

    /* with uniform move costs, we can compute the child distances once */
    child_dist_g = map->cells[ cur_entry.cell ].dist_g + 1;

    /* look at children of cell */
    for( i = 0; i < 4; ++i ) {

      child_entry.cell = make_move( map, cur_entry.cell, i );
      if( child_entry.cell >= 0 ) {
	/* move is valid and child cell is not blocked */

	if( hash_set_contains_key( hash, child_entry.cell )
	    ? child_dist_g < map->cells[ child_entry.cell ].dist_g
	    : ( hash_set_add_key( hash, child_entry.cell ),
		map->cells[ child_entry.cell ].heap_pos = -1, 1 ) ) {
	  /* child was never visited, or new shorter distance */

	  map->cells[ child_entry.cell ].dist_g = child_dist_g;
	  map->cells[ child_entry.cell ].parent = cur_entry.cell;
	  child_entry.dist_f = child_dist_g
	    + mh_dist_cell( map, child_entry.cell, goal );

	  /* add to open list */
	  if( map->cells[ child_entry.cell ].heap_pos >= 0 ) {
	    /* child is already in heap, but we've updated g */

	    heap[ map->cells[ child_entry.cell ].heap_pos ].dist_f
	      = child_entry.dist_f;
	    cell_heapify_up( heap, map,
			     map->cells[ child_entry.cell ].heap_pos );
	  } else {
	    /* child isn't currently in heap */

	    add_to_cell_heap( heap, &in_heap, map, &child_entry );
	  }
	}
      }
    }

    /* pop next cell off heap if not empty */
    if( in_heap == 0 ) {

      break;
    }
    get_from_cell_heap( heap, &in_heap, map, &cur_entry );
  }

  /* done! clean up the heap */
  free( heap );

  /* return the length of the path */
  if( cur_entry.cell == goal ) {

    return map->cells[ cur_entry.cell ].dist_g;
  } else {

    return unreachable;
  }
}


typedef struct {
  distance_t dist_f;
  BlockIdx block;
} block_heap_entry_t;

static inline
void add_to_block_heap( block_heap_entry_t *heap, BlockIdx *in_heap,
			map_t *map, const block_heap_entry_t *entry )
{
  BlockIdx idx;

  idx = *in_heap;
  while( idx && entry->dist_f < heap[ ( idx - 1 ) / 2 ].dist_f ) {

    heap[ idx ] = heap[ ( idx - 1 ) / 2 ];
    map->blocks[ heap[ idx ].block ].heap_pos = idx;
    idx = ( idx - 1 ) / 2;
#ifdef COUNT_HEAP_OPS
    ++g_heap_ops;
#endif
  }
  heap[ idx ] = *entry;
  map->blocks[ heap[ idx ].block ].heap_pos = idx;
  ++( *in_heap );
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif
}

static inline
void block_heapify_up( block_heap_entry_t *heap, map_t *map, BlockIdx idx )
{
  block_heap_entry_t t;

  if( idx == 0 ) {
    return;
  }

  t = heap[ idx ];
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif

  while( t.dist_f < heap[ ( idx - 1 ) / 2 ].dist_f ) {

    heap[ idx ] = heap[ ( idx - 1 ) / 2 ];
    map->blocks[ heap[ idx ].block ].heap_pos = idx;
    idx = ( idx - 1 ) / 2;
#ifdef COUNT_HEAP_OPS
    ++g_heap_ops;
#endif

    if( idx == 0 ) {
      break;
    }
  }

  heap[ idx ] = t;
  map->blocks[ heap[ idx ].block ].heap_pos = idx;
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif
}

static inline
void get_from_block_heap( block_heap_entry_t *heap, BlockIdx *in_heap,
			  map_t *map, block_heap_entry_t *entry )
{
  BlockIdx idx;

  *entry = heap[ 0 ];
  map->blocks[ entry->block ].heap_pos = -1;
#ifdef COUNT_HEAP_OPS
  ++g_heap_ops;
#endif

  --( *in_heap );
  if( *in_heap ) {

    idx = 0;
    while( idx * 2 + 1 < *in_heap ) {

      if( idx * 2 + 2 < *in_heap
	  && heap[ idx * 2 + 1 ].dist_f > heap[ idx * 2 + 2 ].dist_f
	  && heap[ idx * 2 + 2 ].dist_f < heap[ *in_heap ].dist_f ) {

	heap[ idx ] = heap[ idx * 2 + 2 ];
	map->blocks[ heap[ idx ].block ].heap_pos = idx;
	idx = idx * 2 + 2;
#ifdef COUNT_HEAP_OPS
	++g_heap_ops;
#endif
      } else if( heap[ idx * 2 + 1 ].dist_f < heap[ *in_heap ].dist_f ) {

	heap[ idx ] = heap[ idx * 2 + 1 ];
	map->blocks[ heap[ idx ].block ].heap_pos = idx;
	idx = idx * 2 + 1;
#ifdef COUNT_HEAP_OPS
	++g_heap_ops;
#endif
      } else {

	break;
      }
    }

    heap[ idx ] = heap[ *in_heap ];
    map->blocks[ heap[ idx ].block ].heap_pos = idx;
  }
}

/* get path, from goal to start, terminated with -1
   must have sufficient space in path[] for the complete path
   must not be called with unreachable start_cell/goal_cell pairs! */
void block_astar_path( const map_t *map,
					   const pdb_t *pdb,
					   const CellIdx start_cell,
					   const CellIdx goal_cell,
					   CellIdx *path )
{
  int8_t per_idx, entrance, parent_dir, out;
  BlockIdx i;
  distance_t *cur_dists, dist;
  CellIdx path_len, topleft;
  BlockIdx start_block, goal_block, cur;
  pdb_distance_t *pdb_entry;
  distance_t block_dists[ BLOCK_SIZE * BLOCK_SIZE ];
  distance_t goal_dists[ PERIMETER_SIZE ];
  distance_t distances[ BLOCK_SIZE * BLOCK_SIZE ];
  CellIdx parents[ BLOCK_SIZE * BLOCK_SIZE ];

  start_block = block_idx( map, start_cell );
  goal_block = block_idx( map, goal_cell );

  get_block_distances( map, goal_cell, -1, block_dists, NULL );
  for( i = 0; i < PERIMETER_SIZE; ++i ) {
    goal_dists[ i ] = block_dists[ map->perimeter_indices[ i ] ];
  }

  path[ 0 ] = goal_cell;
  path_len = 1;

  /* handle start == goal */
  if( start_cell == goal_cell ) {
    path[ path_len ] = -1;
    return;
  }

  /* find a perimeter cell with the shortest distance to goal cell */
  cur_dists = map->blocks[ goal_block ].dist_g;
  dist = unreachable;
  for( i = 0; i < PERIMETER_SIZE; ++i ) {

    if( goal_dists[ i ] == unreachable
        || cur_dists[ i ] == unreachable ) { continue; }

    if( cur_dists[ i ] + goal_dists[ i ] < dist ) {

      dist = cur_dists[ i ] + goal_dists[ i ];
      per_idx = i;
    }
  }

  /* we'll need the top left cell of the goal block */
  topleft = block_topleft_idx( map, goal_block );

  /* handle the case where the shortest path is all within a single block */
  if( start_block == goal_block ) {

    get_block_distances( map, start_cell, goal_cell, distances, parents );
    i = goal_cell / map->width % BLOCK_SIZE * BLOCK_SIZE
      + goal_cell % BLOCK_SIZE;
    if( distances[ i ] <= dist ) {
      /* start -> goal within the same block */

      /* add any intermediate points to the path */
      while( parents[ i ] >= 0 ) {

	i = parents[ i ];
        path[ path_len ] = topleft
	  + map->block_offsets[ 2 * BLOCK_SIZE * BLOCK_SIZE + i ];
        ++path_len;
      }
      assert( path[ path_len - 1 ] == start_cell );
      path[ path_len ] = -1;
      return;
    }
  }

  /* add any intermediate points on path from the perimeter cell to goal */
  get_block_distances( map, topleft + map->perimeter_offsets[ per_idx ],
		       goal_cell, distances, parents );
  i = goal_cell / map->width % BLOCK_SIZE * BLOCK_SIZE
      + goal_cell % BLOCK_SIZE;
  while( parents[ i ] >= 0 ) {

    i = parents[ i ];
    path[ path_len ] = topleft
      + map->block_offsets[ 2 * BLOCK_SIZE * BLOCK_SIZE + i ];
    ++path_len;
  }

  cur = goal_block; /* and topleft has already been computed for goal */
  while( 1 ) {
    /* put the best perimeter index into the path */
    if( topleft + map->perimeter_offsets[ per_idx ]
        != path[ path_len - 1 ] ) {
      path[ path_len ] = topleft + map->perimeter_offsets[ per_idx ];
      ++path_len;
    }

    /* get parent of of cur[per_idx] */
    if( map->blocks[ cur ].parents[ per_idx ] < 0 ) {
      /* we're done */

      break;
    }

    /* break the parent down into direction and entrance */
    entrance = map->blocks[ cur ].parents[ per_idx ] % BLOCK_SIZE;
    parent_dir = map->blocks[ cur ].parents[ per_idx ] / BLOCK_SIZE;

    /* put any path points into the path */
    out = map->inv_perimeter_in[ parent_dir * PERIMETER_SIZE + per_idx ];
    pdb_entry = &pdb->distances[ ( get_map_block_bits( map, cur, parent_dir )
                                   * BLOCK_SIZE + entrance )
                                 * PERIMETER_SIZE + out ];
    for( i = 0; pdb_entry->path_points[ i ] >= 0; ++i ) {

      path[ path_len ] = topleft
        + map->block_offsets[ parent_dir * BLOCK_SIZE * BLOCK_SIZE
			      + pdb_entry->path_points[ i ] ];
      ++path_len;
    }

    /* figure out the parent block and perimeter index */
    cur = make_block_move( map, cur, parent_dir ^ 2 );
    topleft = block_topleft_idx( map, cur );
    per_idx = map->edge_out[ parent_dir * BLOCK_SIZE + entrance ];
  }

  /* add any intermediate points on path from the start to perimeter vertex */
  get_block_distances( map, start_cell,
		       topleft + map->perimeter_offsets[ per_idx ],
		       distances,
		       parents );
  i = topleft + map->perimeter_offsets[ per_idx ];
  i = i / map->width % BLOCK_SIZE * BLOCK_SIZE
      + i % BLOCK_SIZE;
  while( parents[ i ] >= 0 ) {

    i = parents[ i ];
    path[ path_len ] = topleft
      + map->block_offsets[ 2 * BLOCK_SIZE * BLOCK_SIZE + i ];
    ++path_len;
  }
  assert( path[ path_len - 1 ] == start_cell );
  path[ path_len ] = -1;
}

distance_t block_astar( map_t *map, const pdb_t *pdb,
			const CellIdx start_cell, const CellIdx goal_cell )
{
  static hash_set_t *hash = NULL;
  CellIdx idx;
  BlockIdx start, goal, in_heap;
  block_heap_entry_t *heap, cur_entry, child_entry;
  distance_t goal_dist, block_dists[ BLOCK_SIZE * BLOCK_SIZE ];
  distance_t goal_dists[ PERIMETER_SIZE ], unreachable_dists[ PERIMETER_SIZE ];
  distance_t *cur_dists;
  int8_t i;

  if( start_cell == goal_cell ) {

    return 0;
  }

  /* get start and goal block */
  start = block_idx( map, start_cell );
  goal = block_idx( map, goal_cell );

  /* get perimeter distances for start and goal blocks */
  get_block_distances( map, start_cell, -1, block_dists, NULL );
  for( idx = 0; idx < PERIMETER_SIZE; ++idx ) {

    map->blocks[ start ].dist_g[ idx ]
      = block_dists[ map->perimeter_indices[ idx ] ];
  }
  get_block_distances( map, goal_cell, -1, block_dists, NULL );
  for( idx = 0; idx < PERIMETER_SIZE; ++idx ) {
    goal_dists[ idx ] = block_dists[ map->perimeter_indices[ idx ] ];
  }

  /* create an unreachable perimeter for new blocks */
  for( idx = 0; idx < PERIMETER_SIZE; ++idx ) {

    unreachable_dists[ idx ] = unreachable;
  }

  /* initialise the upper bound on the distance from start to goal */
  if( goal == start ) {

    goal_dist = block_dists[ start_cell / map->width % BLOCK_SIZE * BLOCK_SIZE
			     + start_cell % BLOCK_SIZE ];
  } else {

    goal_dist = unreachable;
  }

  /* make sure we have distance hash table */
  if( hash != NULL ) {

    clear_hash_set( hash );
  } else {

    hash = init_hash_set( 0.75 );
  }

  /* create heap */
  heap = malloc( sizeof( *heap ) * map->size );
  in_heap = 0;

  /* set up as though we just pulled start block from heap */
  cur_entry.block = start;
  /* dist_g has already been set up */
  map->blocks[ cur_entry.block ].heap_pos = -1;
  for( i = 0; i < PERIMETER_SIZE; ++i ) {
    map->blocks[ cur_entry.block ].parents[ i ] = -1;
  }
  hash_set_add_key( hash, cur_entry.block );

  /* search! */
  while( 1 ) {

    /* expanding another block */
    ++g_expanded;

    cur_dists = map->blocks[ cur_entry.block ].dist_g;

    /* goal block needs some extra handling */
    if( cur_entry.block == goal ) {

      /* check distance to goal for each perimeter cell */
      for( i = 0; i < PERIMETER_SIZE; ++i ) {

	if( goal_dists[ i ] == unreachable
	    || cur_dists[ i ] == unreachable ) { continue; }

	if( cur_dists[ i ] + goal_dists[ i ] < goal_dist ) {

	  goal_dist = cur_dists[ i ] + goal_dists[ i ];
	}
      }

      /* can't quit yet, because there may be a shorter path
         through other blocks */
    }

    /* look at children of block */
    for( i = 0; i < 4; ++i ) {

      child_entry.block = make_block_move( map, cur_entry.block, i );
      if( child_entry.block < 0 ) {
	continue;
      }

      if( !hash_set_contains_key( hash, child_entry.block ) ) {

	hash_set_add_key( hash, child_entry.block );
	memcpy( map->blocks[ child_entry.block ].dist_g, unreachable_dists,
		sizeof( unreachable_dists[ 0 ] ) * PERIMETER_SIZE );
	map->blocks[ child_entry.block ].heap_pos = -1;
      }

      child_entry.dist_f
	= update_distances( map, pdb, goal_cell, i,
			    cur_entry.block, cur_dists, child_entry.block,
			    map->blocks[ child_entry.block ].dist_g,
			    map->blocks[ child_entry.block ].parents );
      if( child_entry.dist_f != unreachable ) {
	/* child was never visited, or at least one distance was longer */

	/* add to open list */
	if( map->blocks[ child_entry.block ].heap_pos >= 0 ) {
	  /* child is already in heap */

	  if( child_entry.dist_f
	      < heap[ map->blocks[ child_entry.block ].heap_pos ].dist_f ) {
	    /* update moved us up in the heap */

	    heap[ map->blocks[ child_entry.block ].heap_pos ].dist_f
	      = child_entry.dist_f;
	    block_heapify_up( heap, map,
			      map->blocks[ child_entry.block ].heap_pos );
	  } else {
	    /* nothing to do - we updated a distance, but there was
	       already another cell with a smaller f value */

	  }
	} else {
	  /* child isn't currently in heap */

	  add_to_block_heap( heap, &in_heap, map, &child_entry );
	}
      }
    }

#ifdef DEBUG
    for( idx = 0; idx < in_heap; ++idx ) {
      printf( "heap[ %d ] = block %d, dist_f = %d\n", idx,
	      heap[ idx ].block, heap[ idx ].dist_f );
    }
#endif

    /* done if heap is empty */
    if( in_heap == 0 ) {

      break;
    }

    if( heap[ 0 ].dist_f >= goal_dist ) {
      /* we've already found the goal, and we can't improve path - done! */

      break;
    }

    /* pop next block off heap */
    get_from_block_heap( heap, &in_heap, map, &cur_entry );
  }

  /* done! clean up the heap */
  free( heap );

  /* return the length of the path */
  return goal_dist;
}

void random_map( map_t *map, float prob )
{
  CellIdx i;

  for( i = 0; i < map->size; ++i ) {

    if( (double)random() / (double)RAND_MAX < prob ) {

      map->cells[ i ].is_obstacle = 1;
    } else {

      map->cells[ i ].is_obstacle = 0;
    }
  }

  recompute_cells( map );
}

static void usage()
{
  fprintf( stderr, "usage: search FILE map_filename num_trials [search_algo]\n" );
  fprintf( stderr, "or     search num_trials width height obstacle_prob [search_algo]\n" );
  fprintf( stderr, "where search_algo is one of IDASTAR ASTAR or BLOCK_ASTAR\n" );
}

int check_path( map_t *map, CellIdx start, CellIdx goal, CellIdx *path,
		int64_t dist )
{
  int i;

  if( path[ 0 ] != goal ) {
    return 0;
  }
  for( i = 1; path[ i ] >= 0; ++i ) {
    if( make_move( map, path[ i - 1 ], 0 ) != path[ i ]
	&& make_move( map, path[ i - 1 ], 1 ) != path[ i ]
	&& make_move( map, path[ i - 1 ], 2 ) != path[ i ]
	&& make_move( map, path[ i - 1 ], 3 ) != path[ i ] ) {
      return 0;
    }
  }
  if( path[ i - 1 ] != start ) {
    return 0;
  }
  if( i != dist + 1 ) {
    return 0;
  }
  return 1;
}

//int main( int argc, char **argv )
int old_main(int argc, char **argv)
{
  map_t *map;
  pdb_t *pdb = NULL;

  int use_random_maps, num_trials, search_algorithm = ASTAR_SEARCH;
  CellIdx width, height;
  double obstacle_prob;

  int64_t total_distance, dist, total_expanded;
#ifdef COUNT_HEAP_OPS
  int64_t total_heap_ops;
#endif
  CellIdx start, goal;
  int i, num_unreachable;
  struct timeval total_time, start_time, end_time;

  if( argc < 2 ) {
    usage();
    exit( EXIT_FAILURE );
  }

  if( !strcmp( argv[ 1 ], "FILE" ) ) {
    if( argc < 4 ) {
      usage();
      exit( EXIT_FAILURE );
    }

    map = read_map( argv[ 2 ] );
    if( map == NULL ) {

      fprintf( stderr, "could not read map %s\n", argv[ 2 ] );
      exit( EXIT_FAILURE );
    }

    if( sscanf( argv[ 3 ], "%d", &num_trials ) < 1 || num_trials < 1 ) {

      fprintf( stderr, "bad number of maps: %s\n", argv[ 3 ] );
      exit( EXIT_FAILURE );
    }

    use_random_maps = 0;
    i = 4;
  } else {
    if( argc < 5 ) {
      usage();
      exit( EXIT_FAILURE );
    }

    if( sscanf( argv[ 1 ], "%d", &num_trials ) < 1 || num_trials < 1 ) {

      fprintf( stderr, "bad number of maps: %s\n", argv[ 1 ] );
      exit( EXIT_FAILURE );
    }
    if( sscanf( argv[ 2 ], "%"SCNd32, &width ) < 1 || width < 1 ) {

      fprintf( stderr, "bad width: %s\n", argv[ 2 ] );
      exit( EXIT_FAILURE );
    }
    if( sscanf( argv[ 3 ], "%"SCNd32, &height ) < 1 || height < 1 ) {

      fprintf( stderr, "bad height: %s\n", argv[ 3 ] );
      exit( EXIT_FAILURE );
    }
    if( sscanf( argv[ 4 ], "%lf", &obstacle_prob ) < 1
	|| obstacle_prob < 0.0 || obstacle_prob >= 1.0 ) {

      fprintf( stderr, "bad obstacle probability: %s\n", argv[ 4 ] );
      exit( EXIT_FAILURE );
    }

    map = new_map( width, height );

    use_random_maps = 1;
    i = 5;
  }

  /* check for search algorithm option */
  if( argc > i ) {

    if( !strcmp( argv[ i ], "IDASTAR" ) ) {

      search_algorithm = IDASTAR_SEARCH;
    } else if( !strcmp( argv[ i ], "ASTAR" ) ) {

      search_algorithm = ASTAR_SEARCH;
    } else if( !strcmp( argv[ i ], "BLOCK_ASTAR" ) ) {

      search_algorithm = BLOCK_ASTAR_SEARCH;
    } else {

      fprintf( stderr, "unknown algorithm %s\n", argv[ i ] );
      exit( EXIT_FAILURE );
    }
  }

  if( search_algorithm == BLOCK_ASTAR_SEARCH ) {

    pdb = build_block_pdb();
  }

  total_expanded = 0;
  total_distance = 0;
  total_time.tv_sec = 0;
  total_time.tv_usec = 0;
  num_unreachable = 0;
#ifdef COUNT_HEAP_OPS
  total_heap_ops = 0;
#endif
  for( i = 0; i < num_trials; ++i ) {

    if( use_random_maps ) {
      //srandom( i );  // use i as a seed so we can try a specific random map
      random_map( map, obstacle_prob );
    }

    do {
      start = random() % map->size;
    } while( map->cells[ start ].is_obstacle );
    do {
      goal = random() % map->size;
    } while( map->cells[ goal ].is_obstacle );
#ifdef DEBUG
    print_map( stdout, map );
    printf( "%d,%d %d,%d\n", start % map->width, start / map->width,
	    goal % map->width, goal / map->width );
#endif

    g_expanded = 0;
#ifdef COUNT_HEAP_OPS
    g_heap_ops = 0;
#endif

    gettimeofday( &start_time, NULL );
    if( search_algorithm == IDASTAR_SEARCH ) {

      dist = cell_idastar( map, start, goal );
    } else if( search_algorithm == ASTAR_SEARCH ) {

      dist = cell_astar( map, start, goal );
#ifdef DEBUG
      if( dist < unreachable ) {
	CellIdx path[ dist + 1 ];

	astar_path( map, start, goal, path );
	assert( check_path( map, start, goal, path, dist ) );
      }
#endif
    } else {

      dist = block_astar( map, pdb, start, goal );
    //if( dist != cell_astar( map, start, goal ) ) { printf( "FAILED ON %d\n", i ); exit( -1 ); }
#ifdef DEBUG
      if( dist < unreachable ) {
	CellIdx path[ dist + 1 ];

	block_astar_path( map, pdb, start, goal, path );
	assert( check_path( map, start, goal, path, dist ) );
      }
#endif
    }
    gettimeofday( &end_time, NULL );

    end_time.tv_sec -= start_time.tv_sec;
    end_time.tv_usec -= start_time.tv_usec;
    if( end_time.tv_usec < 0 ) {
      end_time.tv_usec += 1000000;
      --end_time.tv_sec;
    }
#ifdef VERBOSE
    if( dist == unreachable ) {
      printf( "unreachable" );
    } else {
      printf( "distance %"PRId64, dist );
    }
    printf( ", %"PRId64" %s, %zd.%06zds", g_expanded,
	    search_algorithm == BLOCK_ASTAR_SEARCH ? "blocks" : "cells",
	    end_time.tv_sec, end_time.tv_usec );
#ifdef COUNT_HEAP_OPS
    printf( ", %"PRId64" heap ops", g_heap_ops );
#endif
    printf( "\n" );
#endif

    if( dist == unreachable ) {
      ++num_unreachable;
    } else {
      total_distance += dist;
    }
    total_expanded += g_expanded;
    total_time.tv_sec += end_time.tv_sec;
    total_time.tv_usec += end_time.tv_usec;
    total_time.tv_sec += total_time.tv_usec / 1000000;
    total_time.tv_usec %= 1000000;
#ifdef COUNT_HEAP_OPS
    total_heap_ops += g_heap_ops;
#endif
  }

  printf( "total: %"PRId64" distance, %d unreachable, %"PRId64" %s, %zd.%06zds",
	  total_distance, num_unreachable, total_expanded,
	  search_algorithm == BLOCK_ASTAR_SEARCH ? "blocks" : "cells",
	  total_time.tv_sec, total_time.tv_usec );
#ifdef COUNT_HEAP_OPS
  printf( ", %"PRId64" heap ops", total_heap_ops );
#endif
  printf( "\n" );


  destroy_map( map );

  return EXIT_SUCCESS;
}
