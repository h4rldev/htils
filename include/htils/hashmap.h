#ifndef HTILS_HASHMAP_H
#define HTILS_HASHMAP_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

//
//
//

enum hashmap_entry_state {
  EMPTY,
  OCCUPIED,
  DEAD,
};
typedef enum hashmap_entry_state hashmap_entry_state_t;

enum hashmap_result {
  CREATED,
  UPDATED,

  NOT_FOUND,
  KILLED,
};
typedef enum hashmap_result hashmap_result_t;

typedef struct hashmap_entry hashmap_entry_t;
struct hashmap_entry {
  string *key;
  void *value;
  enum hashmap_entry_state state;
};

typedef struct hashmap hashmap_t;
struct hashmap {
  arena_t *arena;
  hashmap_entry_t *entries;

  u64 capacity;
  u64 count;
  u64 dead_entries;
};

//
//
//

#ifdef USE_NULLABLE_TYPES
typedef hashmap_t hashmap_nullable_t;
#endif

//
//
//

/*
 * @brief Creates a new hashmap.
 *
 * @details Creates a new hashmap, with the given capacity, and arena, if the
 * given capacity is 0, it will use the built in default capacity.
 *
 * @param arena The arena to allocate from.
 * @param capacity The capacity of the hashmap.
 *
 * @return A pointer to the new hashmap.
 */
hashmap_t *hashmap_new(arena_t *arena, const u64 capacity);

//
//
//

/*
 * @brief Insert a K/V into the hashmap.
 *
 * @details Inserts a K/V into the hashmap, if the key already exists, it will
 * be updated, if the capacity is too small, it will be automatically grown.
 *
 * @param hashmap The hashmap to insert into.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 *
 * @return The result of the insert (which will be either CREATED or UPDATED in
 * this case).
 */
hashmap_result_t hashmap_insert(hashmap_t *hashmap, const string *key,
                                const void *value);

/*
 * @brief Remove / Kill a K/V from the hashmap.
 *
 * @details Removes a K/V from the hashmap, if the key doesn't exist, it will
 * return NOT_FOUND, otherwise it will return KILLED.
 *
 * @param hashmap The hashmap to remove from.
 * @param key The key to remove.
 *
 * @return The result of the remove (which will be either KILLED or NOT_FOUND in
 * this case).
 */
hashmap_result_t hashmap_kill(hashmap_t *hashmap, const string *key);

//
//
//

/*
 * @brief Get a V from the hashmap.
 *
 * @details Gets an entry from the hashmap, if the entry doesn't exist, it will
 * be null.
 *
 * @param hashmap The hashmap to get from.
 * @param key The key to get.
 *
 * @return The value associated with the key, or null if it doesn't exist or is
 * dead.
 */
void *hashmap_get(hashmap_t *hashmap, const string *key);

#endif // !HTILS_HASHMAP_H
