#ifndef HTILS_HASHMAP_H
#define HTILS_HASHMAP_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

//
//
//

/**
 * @brief The state of a hashmap entry.
 *
 * @param EMPTY The entry is empty.
 * @param OCCUPIED The entry is occupied.
 * @param DEAD The entry is dead.
 */
typedef enum hashmap_entry_state {
  EMPTY,
  OCCUPIED,
  DEAD,
} hashmap_entry_state_t;

/**
 *
 * @brief The result of a hashmap operation.
 *
 * @param CREATED The entry was created.
 * @param UPDATED The entry was updated.
 *
 * @param NOT_FOUND The entry was not found.
 * @param KILLED The entry was killed.
 */
typedef enum hashmap_result {
  CREATED,
  UPDATED,

  NOT_FOUND,
  KILLED,
} hashmap_result_t;

/**
 * @brief A hashmap entry.
 *
 * @param key The key of the entry.
 * @param value The value of the entry.
 * @param state The state of the entry.
 */
typedef struct hashmap_entry {
  string *key;
  void *value;
  enum hashmap_entry_state state;
} hashmap_entry_t;

/**
 * @brief A hashmap.
 *
 * @param arena The \ref arena to allocate from.
 * @param entries The entries of the \ref hashmap.
 * @param capacity The capacity of the \ref hashmap.
 * @param count The amount of entries in the \ref hashmap.
 * @param dead_entries The amount of dead entries of the \ref hashmap.
 */
typedef struct hashmap {
  arena_t *arena;
  hashmap_entry_t *entries;

  u64 capacity;
  u64 count;
  u64 dead_entries;
} hashmap_t;

//
//
//

/** Optionally use a nullable type for hashmaps. */
#ifdef USE_NULLABLE_TYPES
typedef hashmap_t hashmap_nullable_t;
#endif

//
//
//

/**
 * @brief Initializes a new hashmap.
 *
 * @details With the @c capacity, and @c arena, if the
 * given capacity is 0, it will use the built in default capacity, which is
 * `16`.
 *
 * @param arena The \ref arena to allocate from.
 * @param capacity The capacity of the \ref hashmap.
 *
 * @pre @c arena must be valid and cannot be `null`.
 *
 * @return A pointer to the new hashmap.
 */
hashmap_t *hashmap_new(arena_t *arena, const u64 capacity);

//
//
//

/**
 * @brief Insert @c key and @c value into the \ref hashmap.
 *
 * @details Hashes and duplicates the key and sets the value, if the key already
 * exists, it will be updated, if the capacity is too small, it will be
 * automatically grown.
 *
 * @param hashmap The \ref hashmap to insert into.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 *
 * @pre
 * - @c hashmap and @c key must be valid and cannot be `null`.
 * - @c value can be `null`, but there's really no reason to.
 *
 * @return The result of the insert (which will be either CREATED or UPDATED in
 * this case).
 */
hashmap_result_t hashmap_insert(hashmap_t *hashmap, const string *key,
                                const void *value);

/**
 * @brief Remove / Kill entry at @c key from the \ref hashmap.
 *
 * @details Marks an entry as DEAD, if the key doesn't exist, it will
 * return NOT_FOUND, otherwise it will return KILLED.
 *
 * @param hashmap The \ref hashmap to remove from.
 * @param key The key to remove.
 *
 * @pre @c hashmap and @c key must be valid and cannot be `null`.
 *
 * @return The result of the remove (which will be either KILLED or NOT_FOUND in
 * this case).
 */
hashmap_result_t hashmap_kill(hashmap_t *hashmap, const string *key);

//
//
//

/**
 * @brief Get a V from the \ref hashmap.
 *
 * @details Gets an entry from the \ref hashmap, if the entry doesn't exist, it
 * will be null.
 *
 * @param hashmap The \ref hashmap to get from.
 * @param key The key to get.
 *
 * @pre @c hashmap and @c key must be valid and cannot be `null`.
 *
 * @return The value associated with the key, or null if it doesn't exist or is
 * dead.
 */
void *hashmap_get(hashmap_t *hashmap, const string *key);

#endif // !HTILS_HASHMAP_H
