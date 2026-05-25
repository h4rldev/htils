#include <string.h>

//
//
//

#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/basictypes.h>
#include <htils/string.h>
#include <htils/stringmap.h>

#define DEFAULT_CAPACITY 16

//
//
//

stringmap_t *sm_new(arena_t *arena, const u64 capacity) {
  htils_assert(arena != null && "Arena cannot be null.");

  stringmap_t *map = arena_alloc(arena, stringmap_t, 1);
  map->arena = arena;

  if (capacity > 0) {
    map->entries = arena_alloc(arena, stringmap_entry_t, capacity);
    map->capacity = capacity;
  } else {
    map->entries = arena_alloc(arena, stringmap_entry_t, DEFAULT_CAPACITY);
    map->capacity = DEFAULT_CAPACITY;
  }

  for (u64 i = 0; i < map->capacity; i++) {
    map->entries[i].state = EMPTY;
    map->entries[i].key = null;
  }

  map->count = 0;
  map->dead_entries = 0;

  return map;
}

//
//
//

/**
 * @brief Hash a key into an index.
 *
 * @details Hashes a key into an index, using the FNV-1a algorithm.
 *
 * @param key The key to hash.
 *
 * @pre @c key must be valid and cannot be `null`.
 *
 * @return The hash generated from the key.
 */
static u64 hash_key(const string *key) {
  htils_assert(key != null && "Key cannot be null.");
  htils_assert(key->len > 0 && "Key cannot be empty.");

  u64 hash = 14695981039346656037ULL; // FNV-1a

  for (u64 i = 0; i < key->len; i++) {
    hash ^= key->base[i];
    hash *= 1099511628211ULL; // FNV-1a
  }

  return hash;
}

/**
 * @brief Grow a stringmap if it's too small.
 *
 * @details By doubling the capacity, allocating a new block using \ref
 * arena_alloc(), and copying over all occupied entries.
 *
 * @param map The stringmap to grow.
 *
 * @pre @c map must be valid and cannot be `null`.
 */
static void sm_grow(stringmap_t *map) {
  htils_assert(map != null && "stringmap cannot be null.");

  u64 old_capacity = map->capacity;
  stringmap_entry_t *old_entries = map->entries;

  map->capacity *= 2;
  map->entries = arena_alloc(map->arena, stringmap_entry_t, map->capacity);

  for (u64 i = 0; i < old_capacity; i++)
    if (old_entries[i].state == OCCUPIED)
      sm_insert(map, old_entries[i].key, (cstr *)old_entries[i].value);
}

//
//
//

stringmap_result_t __sm_insert(stringmap_t *map, const string *key,
                               const void *value, const u64 vsize) {
  htils_assert(map != null && "stringmap cannot be null.");
  htils_assert(key != null && "Key cannot be null.");

  if ((map->count + map->dead_entries) >= (map->capacity * 0.75))
    sm_grow(map);

  u64 idx = hash_key(key) % map->capacity;
  u64 first_tombstone = UINT64_MAX;

  for (;;) {
    stringmap_entry_t *entry = &map->entries[idx];
    switch (entry->state) {
    case EMPTY:
      if (first_tombstone != UINT64_MAX) {
        idx = first_tombstone;
        entry = &map->entries[idx];
        map->dead_entries--;
      }

      entry->key = string_dup(map->arena, key);
      entry->value = __arena_alloc(map->arena, vsize);
      memcpy(entry->value, value, vsize);
      entry->state = OCCUPIED;
      map->count++;
      return CREATED;

    case DEAD:
      if (first_tombstone == UINT64_MAX)
        first_tombstone = idx;
      break;
    case OCCUPIED:
      if (stringcmp(entry->key, key)) {
        entry->value = __arena_alloc(map->arena, vsize);
        memcpy(entry->value, value, vsize);
        entry->vsize = vsize;
        return UPDATED;
      }
    }
    idx = (idx + 1) % map->capacity;
  }
}

stringmap_result_t sm_kill(stringmap_t *map, const string *key) {
  htils_assert(map != null && "stringmap cannot be null.");
  htils_assert(key != null && "Key cannot be null.");
  htils_assert(key->len > 0 && "Key cannot be empty.");

  u64 idx = hash_key(key) % map->capacity;
  u64 guard = map->capacity;

  while (guard--) {
    stringmap_entry_t *entry = &map->entries[idx];
    switch (entry->state) {
    case EMPTY:
      return NOT_FOUND;
    case DEAD:
      return KILLED;
    case OCCUPIED:
      if (stringcmp(entry->key, key)) {
        entry->state = DEAD;
        entry->key = null;
        entry->value = null;
        entry->vsize = 0;
        map->dead_entries++;
        map->count--;
        return KILLED;
      } else {
        return NOT_FOUND;
      }
    }
    idx = (idx + 1) % map->capacity;
  }
  return NOT_FOUND;
}

//
//
//

void *sm_get(stringmap_t *map, const string *key) {
  htils_assert(map != null && "stringmap cannot be null.");
  htils_assert(key != null && "Key cannot be null.");
  htils_assert(key->len > 0 && "Key cannot be empty.");

  u64 idx = hash_key(key) % map->capacity;
  u64 guard = map->capacity;

  while (guard--) {
    stringmap_entry_t *entry = &map->entries[idx];
    switch (entry->state) {
    case EMPTY:
    case DEAD:
      return null;

    case OCCUPIED:
      if (stringcmp(entry->key, key))
        return entry->value;
      break;
    }
    idx = (idx + 1) % map->capacity;
  }
  return null;
}

/// X3
