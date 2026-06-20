#ifndef H2OTILS_STRINGMAP_H
#define H2OTILS_STRINGMAP_H

#include <h2o/memory.h>

#include <htils/basictypes.h>

#include <h2otils/string.h>

//
//
//

/**
 * @brief The state of a \ref h2o_stringmap_t entry.
 *
 * @param EMPTY The entry is empty.
 * @param OCCUPIED The entry is occupied.
 * @param DEAD The entry is dead.
 */
typedef enum h2o_stringmap_entry_state {
  EMPTY,
  OCCUPIED,
  DEAD,
} h2o_stringmap_entry_state_t;

/**
 *
 * @brief The result of a \ref h2o_stringmap_t operation.
 *
 * @param CREATED The entry was created.
 * @param UPDATED The entry was updated.
 *
 * @param NOT_FOUND The entry was not found.
 * @param KILLED The entry was killed.
 */
typedef enum h2o_stringmap_result {
  CREATED,
  UPDATED,

  NOT_FOUND,
  KILLED,
} h2o_stringmap_result_t;

/**
 * @brief A \ref h2o_stringmap_t entry.
 *
 * @param key The key of the entry.
 * @param value The value of the entry.
 * @param state The state of the entry.
 */
typedef struct h2o_stringmap_entry {
  h2o_string *key;
  void *value;
  u64 vsize;
  enum h2o_stringmap_entry_state state;
} h2o_stringmap_entry_t;

/**
 * @brief A stringmap.
 *
 * @param pool The memory pool to allocate from.
 * @param entries The entries of the \ref h2o_stringmap_t.
 * @param capacity The capacity of the \ref h2o_stringmap_t.
 * @param count The amount of entries in the \ref h2o_stringmap_t.
 * @param dead_entries The amount of dead entries of the \ref h2o_stringmap_t.
 */
typedef struct h2o_stringmap {
  h2o_mem_pool_t *pool;
  h2o_stringmap_entry_t *entries;

  u64 capacity;
  u64 count;
  u64 dead_entries;
} h2o_stringmap_t;

//
//
//

/** Optionally use a nullable type for h2o_stringmaps. */
#ifdef USE_NULLABLE_TYPES
typedef h2o_stringmap_t h2o_stringmap_nullable_t;
#endif

//
//
//

/**
 * @brief Initializes a new \ref h2o_stringmap_t.
 *
 * @details With the @c capacity, and @c pool, if the
 * given capacity is 0, it will use the built in default capacity, which is
 * `16`.
 *
 * @param pool The memory pool to allocate from.
 * @param capacity The capacity of the \ref h2o_stringmap_t.
 *
 * @pre @c pool must be valid and cannot be `null`.
 *
 * @return A pointer to the new \ref h2o_stringmap_t.
 */
h2o_stringmap_t *h2o_sm_new(h2o_mem_pool_t *pool, const u64 capacity);

//
//
//

/**
 * @brief Insert @c key and @c value into the \ref h2o_stringmap_t.
 *
 * @details Hashes and duplicates the key and sets the value, if the key already
 * exists, it will be updated, if the capacity is too small, it will be
 * automatically grown.
 *
 * @note This function is not meant to be run directly, and is called by the
 * \ref h2o_sm_insert() macro.
 *
 * @param map The \ref h2o_stringmap_t to insert into.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 * @param vsize The size of the value.
 *
 * @pre
 * - @c map and @c key must be valid and cannot be `null`.
 * - @c value can't be `null`, as it would lead to a null dereference.
 *
 * @return The result of the insert (which will be either CREATED or UPDATED in
 * this case).
 */
h2o_stringmap_result_t __h2o_sm_insert(h2o_stringmap_t *map,
                                       const h2o_string *key, const void *value,
                                       u64 vsize);

#define __H2O_SM_VAL(val) (val), sizeof(*val)
#define h2o_sm_insert(map, key, value)                                         \
  __h2o_sm_insert(map, key, __H2O_SM_VAL(value))

/**
 * @brief Remove / Kill entry at @c key from the \ref h2o_stringmap_t.
 *
 * @details Marks an entry as DEAD, if the key doesn't exist, it will
 * return NOT_FOUND, otherwise it will return KILLED.
 *
 * @param map The \ref h2o_stringmap_t to remove from.
 * @param key The key to remove.
 *
 * @pre @c stringmap and @c key must be valid and cannot be `null`.
 *
 * @return The result of the remove (which will be either KILLED or NOT_FOUND in
 * this case).
 */
h2o_stringmap_result_t h2o_sm_kill(h2o_stringmap_t *map, const h2o_string *key);

//
//
//

/**
 * @brief Get a V from the \ref h2o_stringmap_t.
 *
 * @details Gets an entry from the \ref h2o_stringmap_t, if the entry doesn't
 * exist, it will be null.
 *
 * @param map The \ref h2o_stringmap_t to get from.
 * @param key The key to get.
 *
 * @pre @c map and @c key must be valid and cannot be `null`.
 *
 * @return The value associated with the key, or null if it doesn't exist or is
 * dead.
 */
void *h2o_sm_get(h2o_stringmap_t *map, const h2o_string *key);

#endif // !H2OTILS_STRINGMAP_H
