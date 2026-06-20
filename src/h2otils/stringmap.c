#include <string.h>

//
//
//

#include <h2o/memory.h>

#include <htils/assert.h>
#include <htils/basictypes.h>

#include <h2otils/string.h>
#include <h2otils/stringmap.h>

#define DEFAULT_CAPACITY 16

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
static u64 hash_key(const h2o_string *key) {
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
 * @brief Inserts a key-value pair into the stringmap directly.
 *
 * @details By first hashing the key, then iterating through the entries.
 *
 * @param map The h2o_stringmap_t to insert into.
 * @param key The key to insert.
 * @param value The value to insert.
 * @param vsize The size of the value.
 *
 * @pre
 *  - @c map, @c key, and @c value must be valid and cannot be `null`.
 *  - @c vsize must be greater than 0 and the explicit size of the value, since
 * @value is a void * it has to be provided.
 */

static void sm_insert_direct(h2o_stringmap_t *map, const h2o_string *key,
                             void *value, u64 vsize) {
  u64 start_idx = hash_key(key) % map->capacity;
  u64 idx = start_idx;

  for (;;) {
    if (idx == start_idx)
      return;

    h2o_stringmap_entry_t *entry = &map->entries[idx];

    if (entry->state != OCCUPIED) {
      entry->key = h2o_string_dup(map->pool, key);
      entry->value = h2o_mem_alloc_pool(map->pool, void *, vsize);
      memcpy(entry->value, value, vsize);

      entry->vsize = vsize;
      entry->state = OCCUPIED;
      map->count++;
      return;
    }

    idx = (idx + 1) % map->capacity;
  }
}

/**
 * @brief Grow a stringmap if it's too small.
 *
 * @details By doubling the capacity, allocating a new block using
 * h2o_mem_alloc_pool(), and copying over all occupied entries.
 *
 * @param map The stringmap to grow.
 *
 * @pre @c map must be valid and cannot be `null`.
 */
static void h2o_sm_grow(h2o_stringmap_t *map) {
  htils_assert(map && "map cannot be null.");

  u64 old_capacity = map->capacity;
  h2o_stringmap_entry_t *old_entries = map->entries;
  u64 old_dead_entries = map->dead_entries;

  map->count = 0;
  map->capacity *= 2;
  map->entries =
      h2o_mem_alloc_pool(map->pool, h2o_stringmap_entry_t, map->capacity);
  for (u64 i = 0; i < map->capacity; ++i) {
    map->entries[i].state = EMPTY;
  }

  for (u64 i = 0; i < old_capacity; ++i) {
    if (old_entries[i].state == OCCUPIED)
      sm_insert_direct(map, old_entries[i].key, old_entries[i].value,
                       old_entries[i].vsize);
  }

  map->dead_entries = old_dead_entries;
}

//
//
//

h2o_stringmap_t *h2o_sm_new(h2o_mem_pool_t *pool, const u64 capacity) {
  htils_assert(pool && "Pool cannot be null.");

  h2o_stringmap_t *map = h2o_mem_alloc_pool(pool, h2o_stringmap_t, 1);
  map->pool = pool;

  if (capacity >= 2) {
    map->entries = h2o_mem_alloc_pool(pool, h2o_stringmap_entry_t, capacity);
    map->capacity = capacity;
  } else {
    map->entries =
        h2o_mem_alloc_pool(pool, h2o_stringmap_entry_t, DEFAULT_CAPACITY);
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

h2o_stringmap_result_t __h2o_sm_insert(h2o_stringmap_t *map,
                                       const h2o_string *key, const void *value,
                                       const u64 vsize) {
  htils_assert(map && "Map cannot be null.");
  htils_assert(key && "Key cannot be null.");
  htils_assert(key->len > 0 && "Key cannot be empty.");
  htils_assert(key->base && "Key base cannot be null.");

  htils_assert(value && "Value cannot be null.");
  htils_assert(vsize > 0 && "Value size cannot be 0.");

  if ((map->count + map->dead_entries) >= (map->capacity * 0.75))
    h2o_sm_grow(map);

  u64 idx = hash_key(key) % map->capacity;
  u64 first_tombstone = UINT64_MAX;

  for (;;) {
    h2o_stringmap_entry_t *entry = &map->entries[idx];
    switch (entry->state) {
    case EMPTY:
      if (first_tombstone != UINT64_MAX) {
        idx = first_tombstone;
        entry = &map->entries[idx];
        map->dead_entries--;
      }

      entry->key = h2o_string_dup(map->pool, key);
      entry->value = h2o_mem_alloc_pool(map->pool, void *, vsize);

      memcpy(entry->value, value, vsize);
      entry->state = OCCUPIED;
      map->count++;
      return CREATED;

    case DEAD:
      if (first_tombstone == UINT64_MAX)
        first_tombstone = idx;
      break;
    case OCCUPIED:
      if (h2o_stringcmp(entry->key, key)) {
        if (entry->vsize >= vsize) {
          memcpy(entry->value, value, vsize);
          entry->vsize = vsize;
        } else {
          entry->value = h2o_mem_alloc_pool(map->pool, void *, vsize);
          memcpy(entry->value, value, vsize);
          entry->vsize = vsize;
        }
        return UPDATED;
      }
    }
    idx = (idx + 1) % map->capacity;
  }
}

h2o_stringmap_result_t h2o_sm_kill(h2o_stringmap_t *map,
                                   const h2o_string *key) {
  htils_assert(map && "stringmap cannot be null.");
  htils_assert(key && "Key cannot be null.");
  htils_assert(key->len > 0 && "Key cannot be empty.");
  htils_assert(key->base && "Key base cannot be null.");

  u64 idx = hash_key(key) % map->capacity;
  u64 guard = map->capacity;

  while (guard--) {
    h2o_stringmap_entry_t *entry = &map->entries[idx];
    switch (entry->state) {
    case EMPTY:
      return NOT_FOUND;
    case DEAD:
      break;
    case OCCUPIED:
      if (h2o_stringcmp(entry->key, key)) {
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

void *h2o_sm_get(h2o_stringmap_t *map, const h2o_string *key) {
  htils_assert(map && "stringmap cannot be null.");
  htils_assert(key && "Key cannot be null.");
  htils_assert(key->len > 0 && "Key cannot be empty.");
  htils_assert(key->base && "Key base cannot be null.");

  u64 start_idx = hash_key(key) % map->capacity;
  u64 idx = start_idx;

  for (;;) {
    h2o_stringmap_entry_t *entry = &map->entries[idx];
    switch (entry->state) {
    case EMPTY:
    case DEAD:
      return null;

    case OCCUPIED:
      if (h2o_stringcmp(entry->key, key))
        return entry->value;
      break;
    }
    idx = (idx + 1) % map->capacity;
    if (idx == start_idx)
      return null;
  }
  return null;
}

/// X3
