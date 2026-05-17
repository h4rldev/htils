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

#define DA_HEADER_SIZE                                                         \
  ((sizeof(da_header_t) + _Alignof(max_align_t) - 1) &                         \
   ~(_Alignof(max_align_t) - 1))

//
//
//

/*
 * @brief Get the da_header
 *
 * @details Gets the da_header from a dynamic array, by casting and then
 * offsetting by the size of it.
 *
 * @param da The dynamic array to get the header from.
 *
 * @return The da_header of the dynamic array.
 */
static inline da_header_t *da__hdr(void *da) {
  return (da_header_t *)((cstr *)da - DA_HEADER_SIZE);
}

/* Gets the length of the current dynamic array */
#define da_len(darray) ((darray) ? da__hdr(darray)->len : 0)

/* Gets the capacity of the current dynamic array */
#define da_cap(darray) ((darray) ? da__hdr(darray)->cap : 0)

/*
 * @brief Initialize a dynamic array.
 *
 * @details Initializes a dynamic array with the given capacity, through some
 * header_size pointer arithmetic.
 *
 * @param arena The arena to allocate the dynamic array from.
 * @param da The dynamic array to initialize.
 * @param intitial_capacity The initial capacity of the dynamic array.
 */
#define da_new(arena, darray, intitial_capacity)                               \
  do {                                                                         \
    u64 capacity = (intitial_capacity);                                        \
    u64 alloc_size = DA_HEADER_SIZE + capacity;                                \
                                                                               \
    da_header_t *header = arena_alloc((arena), darray, alloc_size);            \
    (darray) = (void *)(header + 1);                                           \
  } while (0)

//
//
//

/*
 * @brief Append an item to a dynamic array.
 *
 * @details Appends an item to a dynamic array, if the dynamic array is not
 * large enough, or not initialized, it will create itself, or extend itself.
 *
 * @param arena The arena to allocate the dynamic array from.
 * @param darray The dynamic array to append to.
 * @param item The item to append to the dynamic array.
 */
#define da_append(arena, darray, item)                                         \
  do {                                                                         \
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
    printf("length: %lu\n", da_len(darray));                                   \
    (darray)[da__hdr(darray)->len++] = (item);                                 \
  } while (0)

/*
 * @brief Pop an item from a dynamic array.
 *
 * @details Pops an item from a dynamic array by simply decrementing its length,
 * if the dynamic array is empty, nothing happens.
 *
 * @param darray The dynamic array to pop from.
 */
#define da_pop(darray)                                                         \
  do {                                                                         \
    if (darray && da__hdr(darray)->len > 0)                                    \
      da__hdr(da)->len--;                                                      \
  } while (0)

/*
 * @brief Substitutes with the last item in a dynamic array.
 *
 * @details Substitutes with the last item in a dynamic array, if the dynamic
 * array is empty it will exit.
 *
 * @param darray The dynamic array to get the last item from.
 */
#define da_last(darray)                                                        \
  (htils_assert(da__hdr(darray)->len > 0), (darray)[da__hdr(darray)->len - 1])

/*
 * @brief Clears a dynamic array.
 *
 * @details Clears a dynamic array, setting its length to 0.
 *
 * @param darray The dynamic array to clear.
 */
#define da_clear(darray)                                                       \
  do {                                                                         \
    if (darray)                                                                \
      da__hdr(darray)->len = 0;                                                \
  } while (0)

#endif // !HTILS_DARRAY
