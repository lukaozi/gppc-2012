#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "map.h"


static void init_cells( map_t *map )
{
  CellIdx i;
  int8_t dir;

  for( i = 0; i < map->size; ++i ) {

    map->cells[ i ].is_obstacle = 0;
    for( dir = 0; dir < 4; ++dir ) {

      map->cells[ i ].neighbours[ dir ] = compute_move( map, i, dir );
    }
  }
}

static void init_block_map( map_t *map )
{
  BlockIdx i;
  int8_t dir;

  for( i = 0; i < map->block_map_size; ++i ) {

    for( dir = 0; dir < 4; ++dir ) {

      map->blocks[ i ].bitstrings[ dir ] = 0;
      map->blocks[ i ].neighbours[ dir ] = compute_block_move( map, i, dir );
    }
  }
}

static void init_block_offsets( map_t *map )
{
  CellIdx y, x;

  /* after moving North, South side is the entry edge */
  for( y = 0; y < BLOCK_SIZE; ++y ) {

    for( x = 0; x < BLOCK_SIZE; ++x ) {

      map->block_offsets[ 0 * BLOCK_SIZE * BLOCK_SIZE + y * BLOCK_SIZE + x ]
	= ( BLOCK_SIZE - 1 - y ) * map->width + ( BLOCK_SIZE - 1 - x );
    }
  }

  /* after moving East, West side is the entry edge */
  for( y = 0; y < BLOCK_SIZE; ++y ) {

    for( x = 0; x < BLOCK_SIZE; ++x ) {

      map->block_offsets[ 1 * BLOCK_SIZE * BLOCK_SIZE + y * BLOCK_SIZE + x ]
	= ( BLOCK_SIZE - 1 - x ) * map->width + y;
    }
  }

  /* after moving South, North side is the entry edge (standard orientation) */
  for( y = 0; y < BLOCK_SIZE; ++y ) {

    for( x = 0; x < BLOCK_SIZE; ++x ) {

      map->block_offsets[ 2 * BLOCK_SIZE * BLOCK_SIZE + y * BLOCK_SIZE + x ]
	= y * map->width + x;
    }
  }

  /* after moving West, East side is the entry edge */
  for( y = 0; y < BLOCK_SIZE; ++y ) {

    for( x = 0; x < BLOCK_SIZE; ++x ) {

      map->block_offsets[ 3 * BLOCK_SIZE * BLOCK_SIZE + y * BLOCK_SIZE + x ]
	= x * map->width + ( BLOCK_SIZE - 1 - y );
    }
  }
}

static void init_perimeter_indices( map_t *map )
{
  CellIdx i;

  for( i = 0; i < BLOCK_SIZE - 1; ++i ) {

    /* add North edge */
    map->perimeter_indices[ 0 * ( BLOCK_SIZE - 1 ) + i ] = i;

    /* add East edge */
    map->perimeter_indices[ 1 * ( BLOCK_SIZE - 1 ) + i ]
      = i * BLOCK_SIZE + ( BLOCK_SIZE - 1 );

    /* add South edge */
    map->perimeter_indices[ 2 * ( BLOCK_SIZE - 1 ) + i ]
      = BLOCK_SIZE * BLOCK_SIZE - 1 - i;

    /* add West edge */
    map->perimeter_indices[ 3 * ( BLOCK_SIZE - 1 ) + i ]
      = ( BLOCK_SIZE - 1 - i ) * BLOCK_SIZE;
  }
}

static void init_perimeter_offsets( map_t *map )
{
  CellIdx i;

  for( i = 0; i < PERIMETER_SIZE; ++i ) {

    /* there's no rotation moving South/2, so use block_offsets[ South ] */
    map->perimeter_offsets[ i ]
      = map->block_offsets[ 2 * BLOCK_SIZE * BLOCK_SIZE
			    + map->perimeter_indices[ i ] ];
  }
}

static void init_edge_out( map_t *map )
{
  CellIdx i;

  for( i = 0; i < BLOCK_SIZE; ++i ) {

    /* North */
    map->edge_out[ 0 * BLOCK_SIZE + i ] = BLOCK_SIZE - 1 - i;

    /* East */
    map->edge_out[ 1 * BLOCK_SIZE + i ] = 2 * ( BLOCK_SIZE - 1 ) - i;

    /* South */
    map->edge_out[ 2 * BLOCK_SIZE + i ] = 3 * ( BLOCK_SIZE - 1 ) - i;

    /* West */
    map->edge_out[ 3 * BLOCK_SIZE + i ]
      = ( 4 * ( BLOCK_SIZE - 1 ) - i ) % ( ( BLOCK_SIZE - 1 ) * 4 );
  }
}

static void init_perimeter_in( map_t *map )
{
  CellIdx i;

  for( i = 0; i < PERIMETER_SIZE; ++i ) {

    /* North: South edge rotated to top */
    map->perimeter_in[ 0 * PERIMETER_SIZE + i ]
      = ( i + ( BLOCK_SIZE - 1 ) * 2 ) % PERIMETER_SIZE;

    /* East: West edge rotated to top */
    map->perimeter_in[ 1 * PERIMETER_SIZE + i ]
      = ( i + ( BLOCK_SIZE - 1 ) * 3 ) % PERIMETER_SIZE;

    /* South: North edge rotated to top */
    map->perimeter_in[ 2 * PERIMETER_SIZE + i ] = i;

    /* West: East edge rotated to top */
    map->perimeter_in[ 3 * PERIMETER_SIZE + i ]
      = ( i + ( BLOCK_SIZE - 1 ) * 1 ) % PERIMETER_SIZE;
  }
}

static void init_inv_perimeter_in( map_t *map )
{
  CellIdx i;

  for( i = 0; i < PERIMETER_SIZE; ++i ) {

    /* North: South edge rotated to top */
    map->inv_perimeter_in[ 0 * PERIMETER_SIZE + i ]
      = ( i + ( BLOCK_SIZE - 1 ) * 2 ) % PERIMETER_SIZE;

    /* East: West edge rotated to top */
    map->inv_perimeter_in[ 1 * PERIMETER_SIZE + i ]
      = ( i + ( BLOCK_SIZE - 1 ) * 1 ) % PERIMETER_SIZE;

    /* South: North edge rotated to top */
    map->inv_perimeter_in[ 2 * PERIMETER_SIZE + i ] = i;

    /* West: East edge rotated to top */
    map->inv_perimeter_in[ 3 * PERIMETER_SIZE + i ]
      = ( i + ( BLOCK_SIZE - 1 ) * 3 ) % PERIMETER_SIZE;
  }
}

static void init_modified_edges( map_t *map )
{
  CellIdx i;

  /* clear all bitmasks, as we will incrementally modify them below */
  for( i = 0; i < PERIMETER_SIZE; ++i ) {

    map->modified_edges[ i ] = 0;
  }

  /* North edge */
  for( i = ( BLOCK_SIZE - 1 ) * 0; i <= ( BLOCK_SIZE - 1 ) * 1; ++i ) {

    map->modified_edges[ i ] |= 1 << 0;
  }

  /* East edge */
  for( i = ( BLOCK_SIZE - 1 ) * 1; i <= ( BLOCK_SIZE - 1 ) * 2; ++i ) {

    map->modified_edges[ i ] |= 1 << 1;
  }

  /* South edge */
  for( i = ( BLOCK_SIZE - 1 ) * 2; i <= ( BLOCK_SIZE - 1 ) * 3; ++i ) {

    map->modified_edges[ i ] |= 1 << 2;
  }

  /* West edge */
  for( i = ( BLOCK_SIZE - 1 ) * 3; i <= ( BLOCK_SIZE - 1 ) * 4; ++i ) {

    map->modified_edges[ i % PERIMETER_SIZE ] |= 1 << 3;
  }
}

map_t *new_map( const CellIdx width, const CellIdx height )
{
  map_t *map;

  map = malloc( sizeof( *map ) );
  if( map == NULL ) {
    return NULL;
  }

  map->cells = NULL;
  map->blocks = NULL;
  map->block_offsets = NULL;
  map->perimeter_indices = NULL;
  map->perimeter_offsets = NULL;
  map->edge_out = NULL;
  map->perimeter_in = NULL;
  map->inv_perimeter_in = NULL;
  map->modified_edges = NULL;

  if( width < 0 ) {
    map->width = BLOCK_SIZE;
  } else if( width % BLOCK_SIZE ) {
    map->width = width + BLOCK_SIZE - width % BLOCK_SIZE;
  } else {
    map->width = width;
  }

  if( height < 0 ) {
    map->height = BLOCK_SIZE;
  } else if( height % BLOCK_SIZE ) {
    map->height = height + BLOCK_SIZE - height % BLOCK_SIZE;
  } else {
    map->height = height;
  }
 
  map->size = map->width * map->height;

  map->cells = malloc( sizeof( map->cells[ 0 ] ) * map->size );
  if( map->cells == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_cells( map );

  map->block_map_width = map->width / BLOCK_SIZE;
  map->block_map_height = map->height / BLOCK_SIZE;
  map->block_map_size = map->block_map_width * map->block_map_height;

  map->blocks = malloc( sizeof( map->blocks[ 0 ] ) * map->block_map_size );
  if( map->blocks == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_block_map( map );

  map->block_offsets = malloc( sizeof( map->block_offsets[ 0 ] )
			       * 4 * BLOCK_SIZE * BLOCK_SIZE );
  if( map->block_offsets == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_block_offsets( map );

  map->perimeter_indices = malloc( sizeof( map->perimeter_indices[ 0 ] )
				   * PERIMETER_SIZE );
  if( map->perimeter_indices == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_perimeter_indices( map );

  map->perimeter_offsets = malloc( sizeof( map->perimeter_offsets[ 0 ] )
				   * PERIMETER_SIZE );
  if( map->perimeter_offsets == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_perimeter_offsets( map );

  map->edge_out = malloc( sizeof( map->edge_out[ 0 ] ) * 4 * BLOCK_SIZE );
  if( map->edge_out == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_edge_out( map );

  map->perimeter_in = malloc( sizeof( map->perimeter_in[ 0 ] )
			      * 4 * PERIMETER_SIZE );
  if( map->perimeter_in == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_perimeter_in( map );

  map->inv_perimeter_in = malloc( sizeof( map->inv_perimeter_in[ 0 ] )
				  * 4 * PERIMETER_SIZE );
  if( map->inv_perimeter_in == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_inv_perimeter_in( map );

  map->modified_edges = malloc( sizeof( map->modified_edges[ 0 ] )
				* PERIMETER_SIZE );
  if( map->modified_edges == NULL ) {
    destroy_map( map );
    return NULL;
  }
  init_modified_edges( map );

  return map;
}

map_t *read_map( const char *map_filename )
{
  FILE *file;
  char header_line[ 64 ], *line;
  CellIdx width, height, x, y;
  map_t *map;

  file = fopen( map_filename, "r" );
  if( file == NULL ) {

    return NULL;
  }

  /* find out how big the map is */
  if( !fgets( header_line, 64, file )
      || sscanf( header_line, "%"SCNu32" %"SCNu32, &width, &height ) < 2 ) {

    fclose( file );
    return NULL;
  }
  line = malloc( width + 3 );

  /* make the map (sides be extended to be a multiple of BLOCK_SIZE) */
  map = new_map( width, height );

  /* go through the file specified number of lines */
  for( y = 0; y < height; ++y ) {

    if( !fgets( line, width + 3, file ) || strlen( line ) < width + 1 ) {

      destroy_map( map );
      free( line );
      fclose( file );
      return NULL;
    }

    /* read the obstacles specified in the file for row y */
    for( x = 0; x < width; ++x ) {

      if( line[ x ] == ' ' || line[ x ] == '0'
	  || line[ x ] == '.' || line[ x ] == 'G' ) {

	map_cell( map, x, y ).is_obstacle = 0;
      } else {

	map_cell( map, x, y ).is_obstacle = 1;
      }
    }

    /* extend the width if neccesary */
    for( ; x < map->width; ++x ) {

      map_cell( map, x, y ).is_obstacle = 1;
    }
  }

  /* extend the height if neccessary */
  for( ; y < map->height; ++y ) {

    for( x = 0; x < map->width; ++x ) {

      map_cell( map, x, y ).is_obstacle = 1;
    }
  }

  free( line );
  fclose( file );

  recompute_cells( map );
  return map;
}

void destroy_map( map_t *map )
{
  if( map ) {

    if( map->modified_edges ) {
      free( map->modified_edges );
    }

    if( map->inv_perimeter_in ) {
      free( map->inv_perimeter_in );
    }

    if( map->perimeter_in ) {
      free( map->perimeter_in );
    }

    if( map->edge_out ) {
      free( map->edge_out );
    }

    if( map->perimeter_offsets ) {
      free( map->perimeter_offsets );
    }

    if( map->perimeter_indices ) {
      free( map->perimeter_indices );
    }

    if( map->block_offsets ) {
      free( map->block_offsets );
    }

    if( map->blocks ) {
      free( map->blocks );
    }

    if( map->cells ) {
      free( map->cells );
    }

    free( map );
  }
}

void print_map( FILE *file, map_t *map )
{
  CellIdx x, y;

  fprintf( file, "%"PRIu32" %"PRIu32"\n", map->width, map->height );

  for( y = 0; y < map->height; ++y ) {

    for( x = 0; x < map->width; ++x ) {

      if( map_cell( map, x, y ).is_obstacle ) {

	fprintf( file, "X" );
      } else {

	fprintf( file, " " );
      }
    }

    fprintf( file, "\n" );
  }
}

/* recompute moves for neighbours */
void static recompute_cell_internal( map_t *map, CellIdx cell )
{
  int8_t dir;
  CellIdx child;

  for( dir = 0; dir < 4; ++dir ) {

    child = compute_move( map, cell, dir );
    if( child < 0 ) { continue; }

    if( map->cells[ cell ].is_obstacle ) {

      map->cells[ child ].neighbours[ dir ^ 2 ] = -1;
    } else {

      map->cells[ child ].neighbours[ dir ^ 2 ] = cell;
    }
  }
}
 
static void recompute_map_block( const map_t *map, BlockIdx block )
{
  int8_t dir;

  /* compute bitstring for all 4 directions */
  for( dir = 0; dir < 4; ++dir ) {

    map->blocks[ block ].bitstrings[ dir ]
      = compute_map_block( map, block, dir );
  }
}

void recompute_cell( map_t *map, CellIdx cell )
{
  recompute_cell_internal( map, cell );
  recompute_map_block( map, block_idx( map, cell ) );
}

void recompute_cells( map_t *map )
{
  CellIdx i;

  for( i = 0; i < map->size; ++i ) {

    recompute_cell_internal( map, i );
  }

  for( i = 0; i < map->block_map_size; ++i ) {

    recompute_map_block( map, i );
  }
}


block_bitstring_t compute_map_block( const map_t *map, BlockIdx block,
				     const int8_t dir )
{
  CellIdx cell, i;
  block_bitstring_t bs;

  cell = block_topleft_idx( map, block );

  bs = 0;
  for( i = 0; i < BLOCK_SIZE * BLOCK_SIZE; ++i ) {

    if( map->cells[ cell
		    + map->block_offsets[ (CellIdx)dir * BLOCK_SIZE * BLOCK_SIZE
					  + i ] ].is_obstacle ) {

      bs |= (block_bitstring_t)1 << i;
    }
  }

  return bs;
}


void get_distances( const map_t *map, const CellIdx start,
		    const distance_t start_distance,
		    distance_t *distances, CellIdx *parents )
{
  distance_t distance;
  CellIdx child, cell, *queue, q_head, q_tail;
  int8_t i;

  /* everything starts off unreachable */
  memset( distances, unreachable, sizeof( *distances ) * map->size );

  if( map->cells[ start ].is_obstacle ) {
    /* start is an obstacle */

    return;
  }

  /* create queue */
  queue = malloc( sizeof( *queue ) * map->size );
  assert( queue != NULL );
  q_head = 0;
  q_tail = 0;

  /* start is the first cell to expand */
  distances[ start ] = start_distance;
  parents[ start ] = -1;
  cell = start;

  /* expand cells */
  while( 1 ) {

    distance = distances[ cell ];

    /* look at children of cell */
    for( i = 0; i < 4; ++i ) {

      child = make_move( map, cell, i );
      if( child >= 0 && distance + 1 < distances[ child ] ) {
	/* move is valid, child cell is not blocked, and it has either
	   never been visited, or the previous distance was longer */

	distances[ child ] = distance + 1;
	parents[ child ] = cell;
	queue[ q_tail ] = child;
	++q_tail;
      }
    }

    /* pop next cell off queue if not empty */
    if( q_head == q_tail ) {

      break;
    }
    cell = queue[ q_head ];
    ++q_head;
  }

  /* done! clean up the queue */
  free( queue );
}


#define cellblock_move_north( map_ptr, cellidx ) ((cellidx)/(map_ptr)->width%BLOCK_SIZE?(cellidx)-(map_ptr)->width:-1)
#define cellblock_move_east( map_ptr, cellidx ) (((cellidx)+1)%BLOCK_SIZE?(cellidx)+1:-1)
#define cellblock_move_south( map_ptr, cellidx ) (((cellidx)/(map_ptr)->width+1)%BLOCK_SIZE?(cellidx)+(map_ptr)->width:-1)
#define cellblock_move_west( map_ptr, cellidx ) ((cellidx)%BLOCK_SIZE?(cellidx)-1:-1)
#define make_cellblock_move( map_ptr, cellidx, move ) ((move)==0?cellblock_move_north(map_ptr,cellidx):(move)==1?cellblock_move_east(map_ptr,cellidx):(move)==2?cellblock_move_south(map_ptr,cellidx):cellblock_move_west(map_ptr,cellidx))

void get_block_distances( const map_t *map,
			  const CellIdx start,
			  const CellIdx goal,
			  distance_t distances[ BLOCK_SIZE*BLOCK_SIZE ],
			  CellIdx *parents )
{
  distance_t distance;
  CellIdx child, cell, *queue, q_head, q_tail;
  int8_t i;

  /* everything starts off unreachable */
  memset( distances, unreachable, sizeof( *distances )
	  * BLOCK_SIZE * BLOCK_SIZE );

  if( map->cells[ start ].is_obstacle ) {
    /* start is an obstacle */

    return;
  }

  /* create queue */
  queue = malloc( sizeof( *queue ) * BLOCK_SIZE * BLOCK_SIZE );
  assert( queue != NULL );
  q_head = 0;
  q_tail = 0;

  /* start is the first cell to expand */
  cell = start;
  distances[ cell / map->width % BLOCK_SIZE * BLOCK_SIZE
	     + cell % BLOCK_SIZE ] = 0;
  if( parents ) {

    parents[ cell / map->width % BLOCK_SIZE * BLOCK_SIZE
	     + cell % BLOCK_SIZE ] = -1;
  }

  /* expand cells */
  while( 1 ) {

    distance = distances[ cell / map->width % BLOCK_SIZE * BLOCK_SIZE
			  + cell % BLOCK_SIZE ];

    /* look at children of cell */
    for( i = 0; i < 4; ++i ) {

      child = make_cellblock_move( map, cell, i );
      if( child >= 0 && map->cells[ child ].is_obstacle == 0
	  && distance + 1
	  < distances[ child / map->width % BLOCK_SIZE * BLOCK_SIZE
		       + child % BLOCK_SIZE ] ) {
	/* move is valid, child cell is not blocked, and it has either
	   never been visited, or the previous distance was longer */

	distances[ child / map->width % BLOCK_SIZE * BLOCK_SIZE
		   + child % BLOCK_SIZE ] = distance + 1;
	if( parents ) {

	  parents[ child / map->width % BLOCK_SIZE * BLOCK_SIZE
		   + child % BLOCK_SIZE ]
	    = cell / map->width % BLOCK_SIZE * BLOCK_SIZE
	    + cell % BLOCK_SIZE;
	}
	queue[ q_tail ] = child;
	++q_tail;
      }
    }

    /* pop next cell off queue if not empty */
    if( q_head == q_tail ) {

      break;
    }
    cell = queue[ q_head ];
    ++q_head;

    /* quit if goal reached */
    if( cell == goal ) {

      break;
    }
  }

  /* done filling in block distances! clean up the queue */
  free( queue );
}
