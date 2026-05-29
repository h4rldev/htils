#ifndef H2OTILS_DARRAY
#define H2OTILS_DARRAY

#include <h2o/memory.h>

//
//
//

#include <htils/assert.h>
#include <htils/basictypes.h>

//
//
//

/**
 * @brief The header of a dynamic array.
 *
 * @param cap The capacity of the dynamic array.
 * @param len The length of the dynamic array.
 */
typedef struct h2o_da_header {
  u64 cap;
  u64 len;
} h2o_da_header_t;

//
//
//

/** The size of the header of a dynamic array. */
#define H2O_DA_HEADER_SIZE                                                     \
  ((sizeof(h2o_da_header_t) + _Alignof(max_align_t) - 1) &                     \
   ~(_Alignof(max_align_t) - 1))

//
//
//

/**
 * @brief Get the dynamic array header
 *
 * @details Casts the da to `cstr *` from a dynamic array, by then
 * offsetting by the size of it.
 *
 * @param da The dynamic array to get the header from.
 *
 * @pre @c da must be valid and cannot be `null`.
 *
 * @return The header of the dynamic array.
 */
static inline h2o_da_header_t *h2o_da__hdr(void *h2o_da) {
  return (h2o_da_header_t *)((cstr *)h2o_da - H2O_DA_HEADER_SIZE);
}

/** Gets the length of the current dynamic array */
#define h2o_da_len(darray) ((darray) ? h2o_da__hdr(darray)->len : 0)

/** Gets the capacity of the current dynamic array */
#define h2o_da_cap(darray) ((darray) ? h2o_da__hdr(darray)->cap : 0)

/**
 * @brief Initialize a dynamic array.
 *
 * @details Caluclates the size for the header with \ref H2O_DA_HEADER_SIZE, and
 * the capacity; Allocates the header with @c initial_capacity, then sets the
 * dynamic array to be the header + 1 offsetting the header.
 *
 * @param pool The memory pool to allocate the dynamic array from.
 * @param darray The dynamic array to initialize.
 * @param intitial_capacity The initial capacity of the dynamic array.
 *
 * @pre
 * - @c pool, and @c darray must be valid and cannot be `null`.
 * - @c intitial_capacity must be greater than 0.
 */
#define h2o_da_new(pool, darray, intitial_capacity)                            \
  do {                                                                         \
    htils_assert(pool != null && "pool cannot be null.");                      \
    htils_assert(intitial_capacity > 0 &&                                      \
                 "Initial capacity must be greater than 0");                   \
                                                                               \
    u64 capacity = (intitial_capacity);                                        \
    u64 alloc_size = H2O_DA_HEADER_SIZE + capacity;                            \
                                                                               \
    h2o_da_header_t *header = h2o_mem_alloc_pool((pool), darray, alloc_size);  \
    header->cap = capacity;                                                    \
    header->len = 0;                                                           \
    (darray) = (void *)(header + 1);                                           \
  } while (0)

//
//
//

/**
 * @brief Append an item to a dynamic array.
 *
 * @details By incrementing the len, has automatic capacity growing
 * functionality.
 *
 * @param pool The memory pool to allocate the dynamic array from.
 * @param darray The dynamic array to append to.
 * @param item The item to append to the dynamic array.
 *
 * @pre @c pool, @c darray, and @c item must be valid and cannot be
 * `null`.
 */
#define h2o_da_append(pool, darray, item)                                      \
  do {                                                                         \
    htils_assert(pool != null && "Pool cannot be null.");                      \
    htils_assert(darray != null && "Darray cannot be null.");                  \
    htils_assert(item != null && "Item cannot be null.");                      \
                                                                               \
    if (!(darray) || h2o_da_len(darray) >= h2o_da_cap(darray)) {               \
      u64 old_capacity = h2o_da_cap(darray);                                   \
      u64 new_capacity =                                                       \
          old_capacity ? old_capacity + old_capacity / 2 + (old_capacity % 2)  \
                       : 8;                                                    \
      u64 old_len = h2o_da_len(darray);                                        \
      u64 alloc_size = H2O_DA_HEADER_SIZE + new_capacity;                      \
                                                                               \
      h2o_da_header_t *header =                                                \
          h2o_mem_alloc_pool((pool), darray, alloc_size);                      \
                                                                               \
      header->cap = new_capacity;                                              \
      header->len = old_len;                                                   \
                                                                               \
      if (darray)                                                              \
        memcpy(header + 1, (darray), old_len * sizeof(*(darray)));             \
                                                                               \
      (darray) = (void *)(header + 1);                                         \
    }                                                                          \
                                                                               \
    (darray)[h2o_da__hdr(darray)->len++] = (item);                             \
  } while (0)

/**
 * @brief Pop an item from a dynamic array.
 *
 * @details By simply decrementing its length, if the dynamic array is empty,
 * nothing happens.
 *
 * @param darray The dynamic array to pop from.
 */
#define h2o_da_pop(darray)                                                     \
  do {                                                                         \
    if (darray && h2o_da__hdr(darray)->len > 0)                                \
      h2o_da__hdr(darray)->len--;                                              \
  } while (0)

/**
 * @brief Substitute with the last item in a dynamic array.
 *
 * @details Asserts that the length of the dynamic array is larger than 0, then
 * simply gets the last entry.
 *
 * @param darray The dynamic array to get the last item from.
 */
#define h2o_da_last(darray)                                                    \
  (htils_assert(h2o_da__hdr(darray)->len > 0),                                 \
   (darray)[h2o_da__hdr(darray)->len - 1])

/**
 * @brief Clears a dynamic array.
 *
 * @details By setting its length to 0, if the dynamic array is null, nothing
 * happens.
 *
 * @param darray The dynamic array to clear.
 */
#define h2o_da_clear(darray)                                                   \
  do {                                                                         \
    if (darray)                                                                \
      h2o_da__hdr(darray)->len = 0;                                            \
  } while (0)

#endif // !H2OTILS_DARRAY
