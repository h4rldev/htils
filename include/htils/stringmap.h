#ifndef HTILS_STRINGMAP_H
#define HTILS_STRINGMAP_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

//
//
//

/**
 * @brief The state of a stringmap entry.
 *
 * @param EMPTY The entry is empty.
 * @param OCCUPIED The entry is occupied.
 * @param DEAD The entry is dead.
 */
typedef enum stringmap_entry_state {
  EMPTY,
  OCCUPIED,
  DEAD,
} stringmap_entry_state_t;

/**
 *
 * @brief The result of a stringmap operation.
 *
 * @param CREATED The entry was created.
 * @param UPDATED The entry was updated.
 *
 * @param NOT_FOUND The entry was not found.
 * @param KILLED The entry was killed.
 */
typedef enum stringmap_result {
  CREATED,
  UPDATED,

  NOT_FOUND,
  KILLED,
} stringmap_result_t;

/**
 * @brief A stringmap entry.
 *
 * @param key The key of the entry.
 * @param value The value of the entry.
 * @param state The state of the entry.
 */
typedef struct stringmap_entry {
  string *key;
  void *value;
  u64 vsize;
  enum stringmap_entry_state state;
} stringmap_entry_t;

/**
 * @brief A stringmap.
 *
 * @param arena The \ref arena to allocate from.
 * @param entries The entries of the \ref stringmap.
 * @param capacity The capacity of the \ref stringmap.
 * @param count The amount of entries in the \ref stringmap.
 * @param dead_entries The amount of dead entries of the \ref stringmap.
 */
typedef struct stringmap {
  arena_t *arena;
  stringmap_entry_t *entries;

  u64 capacity;
  u64 count;
  u64 dead_entries;
} stringmap_t;

//
//
//

/** Optionally use a nullable type for stringmaps. */
#ifdef USE_NULLABLE_TYPES
typedef stringmap_t stringmap_nullable_t;
#endif

//
//
//

/**
 * @brief Initializes a new stringmap.
 *
 * @details With the @c capacity, and @c arena, if the
 * given capacity is 0, it will use the built in default capacity, which is
 * `16`.
 *
 * @param arena The \ref arena to allocate from.
 * @param capacity The capacity of the \ref stringmap.
 *
 * @pre @c arena must be valid and cannot be `null`.
 *
 * @return A pointer to the new stringmap.
 */
stringmap_t *sm_new(arena_t *arena, const u64 capacity);

//
//
//

/**
 * @brief Insert @c key and @c value into the \ref stringmap.
 *
 * @details Hashes and duplicates the key and sets the value, if the key already
 * exists, it will be updated, if the capacity is too small, it will be
 * automatically grown.
 *
 * @note This function is not meant to be run directly, and is called by the
 * \ref sm_ insert() macro.
 *
 * @param stringmap The \ref stringmap to insert into.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 * @param vsize The size of the value.
 *
 * @pre
 * - @c stringmap and @c key must be valid and cannot be `null`.
 * - @c value can't be `null`, as it would leave to a null dereference.
 *
 * @return The result of the insert (which will be either CREATED or UPDATED in
 * this case).
 */
stringmap_result_t __sm_insert(stringmap_t *map, const string *key,
                               const void *value, u64 vsize);

#define SM_VAL(val) (val), sizeof(*val)
#define sm_insert(map, key, value) __sm_insert(map, key, SM_VAL(value))

/**
 * @brief Remove / Kill entry at @c key from the \ref stringmap.
 *
 * @details Marks an entry as DEAD, if the key doesn't exist, it will
 * return NOT_FOUND, otherwise it will return KILLED.
 *
 * @param stringmap The \ref stringmap to remove from.
 * @param key The key to remove.
 *
 * @pre @c stringmap and @c key must be valid and cannot be `null`.
 *
 * @return The result of the remove (which will be either KILLED or NOT_FOUND in
 * this case).
 */
stringmap_result_t sm_kill(stringmap_t *map, const string *key);

//
//
//

/**
 * @brief Get a V from the \ref stringmap.
 *
 * @details Gets an entry from the \ref stringmap, if the entry doesn't exist,
 * it will be null.
 *
 * @param stringmap The \ref stringmap to get from.
 * @param key The key to get.
 *
 * @pre @c stringmap and @c key must be valid and cannot be `null`.
 *
 * @return The value associated with the key, or null if it doesn't exist or is
 * dead.
 */
void *sm_get(stringmap_t *map, const string *key);

#endif // !HTILS_stringmap_H
