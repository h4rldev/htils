#ifndef HTILS_DARRAY
#define HTILS_DARRAY

#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/basictypes.h>

typedef struct da_header da_header_t;
struct da_header {
  u64 cap;
  u64 len;
};

//
//
//

/** The size of the header of a dynamic array. */
#define DA_HEADER_SIZE                                                         \
  ((sizeof(da_header_t) + _Alignof(max_align_t) - 1) &                         \
   ~(_Alignof(max_align_t) - 1))

//
//
//

/**
 * @brief Get the da_header
 *
 * @details Casts the da to `cstr *` from a dynamic array, by then
 * offsetting by the size of it.
 *
 * @param da The dynamic array to get the header from.
 *
 * @pre @c da must be valid and cannot be `null`.
 *
 * @return The da_header of the dynamic array.
 */
static inline da_header_t *da__hdr(void *da) {
  return (da_header_t *)((cstr *)da - DA_HEADER_SIZE);
}

/** Gets the length of the current dynamic array */
#define da_len(darray) ((darray) ? da__hdr(darray)->len : 0)

/** Gets the capacity of the current dynamic array */
#define da_cap(darray) ((darray) ? da__hdr(darray)->cap : 0)

/**
 * @brief Initialize a dynamic array.
 *
 * @details Caluclates the size for the header with \ref DA_HEADER_SIZE, and the
 * capacity; Allocates the header with @c initial_capacity, using \ref
 * arena_alloc(), then sets the dynamic array to be the header + 1 offsetting
 * the header.
 *
 * @param arena The \ref arena to allocate the dynamic array from.
 * @param darray The dynamic array to initialize.
 * @param intitial_capacity The initial capacity of the dynamic array.
 *
 * @pre
 * - @c arena, and @c darray must be valid and cannot be `null`.
 * - @c intitial_capacity must be greater than 0.
 *
 * @see \ref arena_alloc()
 */
#define da_new(arena, darray, intitial_capacity)                               \
  do {                                                                         \
    htils_assert(arena != null && "Arena cannot be null.");                    \
    htils_assert(intitial_capacity > 0 &&                                      \
                 "Initial capacity must be greater than 0");                   \
                                                                               \
    u64 capacity = (intitial_capacity);                                        \
    u64 alloc_size = DA_HEADER_SIZE + capacity;                                \
                                                                               \
    da_header_t *header = arena_alloc((arena), darray, alloc_size);            \
    header->cap = capacity;                                                    \
    (darray) = (void *)(header + 1);                                           \
  } while (0)

//
//
//

/**
 * @brief Append an item to a dynamic array.
 *
 * @details By incrementing the len, has automatic capacity growing
 * functionality through extra allocations with \ref arena_alloc().
 *
 * @param arena The \ref arena to allocate the dynamic array from.
 * @param darray The dynamic array to append to.
 * @param item The item to append to the dynamic array.
 *
 * @pre @c arena, @c darray, and @c item must be valid and cannot be
 * `null`.
 *
 * @see \ref arena_alloc()
 */
#define da_append(arena, darray, item)                                         \
  do {                                                                         \
    htils_assert(arena != null && "Arena cannot be null.");                    \
    htils_assert(darray != null && "Darray cannot be null.");                  \
    htils_assert(item != null && "Item cannot be null.");                      \
                                                                               \
    if (!(darray) || da__hdr(darray)->len >= da__hdr(darray)->cap) {           \
      u64 old_capacity = da__hdr(darray)->cap;                                 \
      u64 new_capacity = old_capacity ? old_capacity * 2 : 8;                  \
      u64 old_len = da__hdr(darray)->len;                                      \
      u64 alloc_size = DA_HEADER_SIZE + new_capacity;                          \
                                                                               \
      da_header_t *header = arena_alloc((arena), darray, alloc_size);          \
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
    (darray)[da__hdr(darray)->len++] = (item);                                 \
  } while (0)

/**
 * @brief Pop an item from a dynamic array.
 *
 * @details By simply decrementing its length, if the dynamic array is empty,
 * nothing happens.
 *
 * @param darray The dynamic array to pop from.
 */
#define da_pop(darray)                                                         \
  do {                                                                         \
    if (darray && da__hdr(darray)->len > 0)                                    \
      da__hdr(darray)->len--;                                                  \
  } while (0)

/**
 * @brief Substitute with the last item in a dynamic array.
 *
 * @details Asserts that the length of the dynamic array is larger than 0, then
 * simply gets the last entry.
 *
 * @param darray The dynamic array to get the last item from.
 */
#define da_last(darray)                                                        \
  (htils_assert(da__hdr(darray)->len > 0), (darray)[da__hdr(darray)->len - 1])

/**
 * @brief Clears a dynamic array.
 *
 * @details By setting its length to 0, if the dynamic array is null, nothing
 * happens.
 *
 * @param darray The dynamic array to clear.
 */
#define da_clear(darray)                                                       \
  do {                                                                         \
    if (darray)                                                                \
      da__hdr(darray)->len = 0;                                                \
  } while (0)

#endif // !HTILS_DARRAY
