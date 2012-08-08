#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MAP_H
#define _MAP_H

#include <inttypes.h>


/* directions = N/0, E/1, S/2, W/3

   a distance of unreachable (defined below) means infinity/unreachable

   assuming blocks with width/height of 5 for the examples below

   01234	block perimeter
   FXXX5
   EXXX6
   DXXX7
   CBA98


   01234	edge distances from parent block
   -----
   01234	child block
   FXXX5
   EXXX6
   DXXX7
   CBA98

   so edge distances 0-4 come from parent block perimeter indices
   { 4, 3, 2, 1, 0 } for a move of direction N/0
   { 8, 7, 6, 5, 4 } for a move of direction E/1
   { C, B, A, 9, 8 } for a move of direction S/2
   { 0, F, E, D, C } for a move of direction W/3
*/

/* a block is BLOCK_SIZE by BLOCK_SIZE cells */
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 5
#endif
/* number of cells in perimeter of a block */
#define PERIMETER_SIZE ((BLOCK_SIZE-1)*4)


typedef int32_t CellIdx;
typedef uint32_t distance_t;
#define unreachable UINT32_MAX
typedef int32_t BlockIdx;

/* a bitstring of obstructions in a block, stored in an integer type */
#if BLOCK_SIZE*BLOCK_SIZE > 63
#error BLOCK_SIZE is too large
#elsif BLOCK_SIZE*BLOCK_SIZE > 31
typedef uint64_t block_bitstring_t;
#else
typedef uint32_t block_bitstring_t;
#endif


typedef struct {
  int8_t is_obstacle;
  CellIdx neighbours[ 4 ];
  distance_t dist_g;
  CellIdx heap_pos;
  CellIdx parent;
} cell_entry_t;

typedef struct {
  /* bitstring[ dir ] gives the bitstring describing the obstacles in
     block (0=clear/1=obstacle), rotated so that the top is adjacent
     to a neighbour making a move of direction dir */
  block_bitstring_t bitstrings[ 4 ];
  BlockIdx neighbours[ 4 ];
  distance_t dist_g[ PERIMETER_SIZE ];
  BlockIdx heap_pos;
  /* parent_dir * ( BLOCK_SIZE + 1 ) + child_entry */
  BlockIdx parents[ PERIMETER_SIZE ];
} block_entry_t;

typedef struct {
  CellIdx width;
  CellIdx height;
  CellIdx size; /* width * height - save some typing and computation time */

  BlockIdx block_map_width;	/* width / BLOCK_SIZE */
  BlockIdx block_map_height;	/* height / BLOCK_SIZE */
  BlockIdx block_map_size;	/* block_map_width * block_map_height */

  cell_entry_t *cells;

  block_entry_t *blocks;


 /* block_offsets[ dir * BLOCK_SIZE * BLOCK_SIZE + i ] gives the i'th
    cell index offset, relative to the un-rotated top left index of
    a block, if it is rotated so that the top is adjacent to a
    neighbour making a move of direction dir.  For example, the North
    side is on top (we're un-rotated) if the move was South */
  CellIdx *block_offsets;

  /* perimeter_indices[ perimeter_index ] gives the cell index
     of the perimeter cell, for a BLOCK_SIZE * BLOCK_SIZE map */
  CellIdx *perimeter_indices;

  /* perimeter_indices[ perimeter_index ] gives the offset of the
     perimeter cell relative to the top left cell (similar to
     block_offsets, although without the rotation) */
  CellIdx *perimeter_offsets;

  /* edge_out[ dir * BLOCK_SIZE + edge_index ] gives the unrotated
     perimeter index for the BLOCK_SIZE cells on an edge, when moving
     in direction dir, ordered in a suitable fashion for use with
     distances (ie they'll be ordeded so thay have been rotated to
     suit the neighbour's orientation, with their entrance edge at the
     North, not the orientation of the source block) */
  CellIdx *edge_out;

  /* perimeter_in[ dir * PERIMETER_SIZE + perimeter_index ]
     gives the perimeter index in the un-rotated block for the cell
     corresponding to perimeter_index in the rotated block.  EG if we
     want perimeter index 5 in a block that was reached by making a
     move of direction South/2, we would need to look at index
     perimeter_in[ 2 * PERIMETER_SIZE + 5 ] */
  CellIdx *perimeter_in;

  /* inv_perimeter_in[ dir * PERIMETER_SIZE + unrotated_index ]
     gives the perimeter index in the rotated block for the cell
     corresponding to perimeter_index in the un-rotated block.  That is,
     it gives the rotated index which corresponds to unrotated_index
     when looking in perimeter_in[ dir * PERIMETER_SIZE + perimeter_index ] */
  CellIdx *inv_perimeter_in;

  /* modified_edges[ perimeter_index ] gives a bitmask describing
     which edges have new values if the value for perimiter cell
     perimiter_index is changed */
  uint8_t *modified_edges;
} map_t;


/* create an empty map with the specified size
   returns NULL on failure */
map_t *new_map( const CellIdx width, const CellIdx height );

/* read a map from a file, returns NULL on failure */
map_t *read_map( const char *map_filename );

/* free memory associated with a map */
void destroy_map( map_t *map );

/* print a map to a file, in a format suitable for read_map */
void print_map( FILE *file, map_t *map );

#define map_cell( map_ptr, x, y ) ((map_ptr)->cells[(CellIdx)(y)*(CellIdx)(map_ptr)->width+(x)])
#define map_cell_index( map_ptr, x, y ) ((CellIdx)(y)*(CellIdx)(map_ptr)->width+(x))
#define map_index_x( map_ptr, index ) (index%((map_ptr)->width))
#define map_index_y( map_ptr, index ) (index/((map_ptr)->width))

/* recompute neighbours and block bitsting after changing a cell */
void recompute_cell( map_t *map, CellIdx cell );

/* recompute neighbours and block bitsting after changing a cell */
void recompute_cells( map_t *map );


/* returns child index, or -1 if it moves off the edge
   doesn't check obstacles! */
#define compute_move_north( map_ptr, cellidx ) ((cellidx)>=(map_ptr)->width?(cellidx)-(map_ptr)->width:-1)
#define compute_move_east( map_ptr, cellidx ) (((cellidx)+1)%(map_ptr)->width?(cellidx)+1:-1)
#define compute_move_south( map_ptr, cellidx ) ((cellidx)+(map_ptr)->width<(map_ptr)->size?(cellidx)+(map_ptr)->width:-1)
#define compute_move_west( map_ptr, cellidx ) ((cellidx)%(map_ptr)->width?(cellidx)-1:-1)
#define compute_move( map_ptr, cellidx, move ) ((move)==0?compute_move_north(map_ptr,cellidx):(move)==1?compute_move_east(map_ptr,cellidx):(move)==2?compute_move_south(map_ptr,cellidx):compute_move_west(map_ptr,cellidx))

/* returns child index, or -1 if it moves off the edge or onto an obstacle */
#define make_move( map_ptr, cellidx, move ) ((map_ptr)->cells[(cellidx)].neighbours[(move)])

/* NOTE: PARAMETERS ARE EVALUATED MULTIPLE TIMES!!! */ 
#define mh_dist_xy( x1, y1, x2, y2 ) (((x1)<(x2)?(x2)-(x1):(x1)-(x2))+((y1)<(y2)?(y2)-(y1):(y1)-(y2)))
#define mh_dist_cell( map_ptr, cell1, cell2 ) (abs((cell1)/(map_ptr)->width-(cell2)/(map_ptr)->width)+abs((cell1)%(map_ptr)->width-(cell2)%(map_ptr)->width))

/* starting from start cell, with the given start distance, find
   distance to all cells, storing the values in distances
   and path information in parents.
   a value of 'unreachable' is used to indicate unreachable cells */
void get_distances( const map_t *map, const CellIdx start,
		    const distance_t start_distance,
		    distance_t *distances, CellIdx *parents );


/* get the block index containing a cell */
/* NOTE: PARAMETERS ARE EVALUATED MULTIPLE TIMES!!! */
#define block_idx( map_ptr, cellidx ) ((cellidx)/(map_ptr)->width/BLOCK_SIZE*(map_ptr)->block_map_width+((cellidx)%(map_ptr)->width)/BLOCK_SIZE)

/* cell index of top left cell in block */
#define block_topleft_idx( map_ptr, blockidx ) (((blockidx)/(map_ptr)->block_map_width*(map_ptr)->width+(blockidx)%(map_ptr)->block_map_width)*BLOCK_SIZE)

/* get a bitstring describing the block containing cell, as approached
   by making a move of direction dir */
/* get the precomputed bitstring for block from the map */
#define get_map_block_bits( map_ptr, block, dir ) ((map_ptr)->blocks[(block)].bitstrings[(dir)])

/* compute the bitstring for block (used for recomputing after changes) */
block_bitstring_t compute_map_block( const map_t *map, BlockIdx block,
				     const int8_t dir );

/* returns child index, or -1 if the move is not possible */
#define compute_block_move_north( map_ptr, blockidx ) ((blockidx)>=(map_ptr)->block_map_width?(blockidx)-(map_ptr)->block_map_width:-1)
#define compute_block_move_east( map_ptr, blockidx ) (((blockidx)+1)%(map_ptr)->block_map_width?(blockidx)+1:-1)
#define compute_block_move_south( map_ptr, blockidx ) ((blockidx)+(map_ptr)->block_map_width<(map_ptr)->block_map_size?(blockidx)+(map_ptr)->block_map_width:-1)
#define compute_block_move_west( map_ptr, blockidx ) ((blockidx)%(map_ptr)->block_map_width?(blockidx)-1:-1)
#define compute_block_move( map_ptr, blockidx, move ) ((move)==0?compute_block_move_north(map_ptr,blockidx):(move)==1?compute_block_move_east(map_ptr,blockidx):(move)==2?compute_block_move_south(map_ptr,blockidx):compute_block_move_west(map_ptr,blockidx))

#define make_block_move( map_ptr, blockidx, move ) ((map_ptr)->blocks[(blockidx)].neighbours[(move)])

/* find distance from start to all cells in the block containing 'start'
   if 'goal' is non-negative, search will stop if cell 'goal' is reached
   if parents is not NULL, set the parents field to recover path */
void get_block_distances( const map_t *map,
			  const CellIdx start,
			  const CellIdx goal,
			  distance_t distances[ BLOCK_SIZE*BLOCK_SIZE ],
			  CellIdx *parents );

#endif

#ifdef __cplusplus
}
#endif
