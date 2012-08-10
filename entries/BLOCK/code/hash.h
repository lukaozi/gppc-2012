#ifdef __cplusplus
extern "C" {
#endif

#ifndef _HASH_H
#define _HASH_H

#include "map.h"


typedef struct {
  int entry_size;
  float max_load;
  CellIdx max_entry;	/* size - 1, size must be a power of 2 */
  int num_entries;
  CellIdx *keys;
  void *entries;
} hash_map_t;

typedef struct {
  float max_load;
  CellIdx max_entry;	/* size - 1, size must be a power of 2 */
  int num_entries;
  CellIdx *keys;
} hash_set_t;


#define hash_map_get_index_key( map_ptr, i ) ( (map_ptr)->keys[ i ] )
#define hash_map_get_index_entry( map_ptr, i ) ( (map_ptr)->entries + (map_ptr)->entry_size * (i) )

hash_map_t *init_hash_map( int entry_size, float max_load );
void delete_hash_map( hash_map_t *map );
void clear_hash_map( hash_map_t *map );
/* if key is already in the table, new info is stored in current entry */
void hash_map_add_entry( hash_map_t *map,
			 const CellIdx key, const void *entry );
/* returns NULL if entry with key is not in hash table */
void *hash_map_get_entry( hash_map_t *map, const CellIdx key );


#define hash_set_get_index_key( set_ptr, i ) ( (set_ptr)->keys[ i ] )

hash_set_t *init_hash_set( float max_load );
void delete_hash_set( hash_set_t *set );
void clear_hash_set( hash_set_t *set );
void hash_set_add_key( hash_set_t *set, const CellIdx key );
/* returns NULL if entry with key is not in hash table */
int8_t hash_set_contains_key( hash_set_t *set, const CellIdx key );

#endif

#ifdef __cplusplus
}
#endif
