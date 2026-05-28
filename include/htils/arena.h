#ifndef HTILS_ARENA_H
#define HTILS_ARENA_H

//
//
//

#include <htils/basictypes.h>

#define KiB(bytes) ((u64)bytes << 10)
#define MiB(bytes) ((u64)bytes << 20)
#define GiB(bytes) ((u64)bytes << 30)

//
//
//

/**
 * @brief An arena.
 *
 * @param reserved The size of the arena.
 * @param committed The size of the committed heap.
 * @param pos The current position of the heap.
 * @param commit_pos The current position of the committed heap.
 */
typedef struct arena {
  u64 reserved;
  u64 committed;
  u64 pos;
  u64 commit_pos;
} arena_t;

/**
 * @brief A temporary arena.
 *
 * @param arena The arena to create the temporary arena from.
 * @param start_pos The position to start the temporary arena from.
 */
typedef struct temp_arena {
  struct arena *arena;
  u64 start_pos;
} temp_arena_t;

//
//
//

/**
 * @brief Create a new \ref arena.
 *
 * @details Initializes a new \ref arena using <a
 * href="https://man7.org/linux/man-pages/man2/mmap.2.html"
 * target="_blank">mmap()</a> or <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc"
 * target="_blank">VirtualAlloc()</a>, depending on system.
 *
 * @param reserve_size The size of the \ref arena to reserve.
 * @param commit_size The size of the \ref arena to commit.
 *
 * @pre
 * - @c reserve_size must be greater than 0.
 * - @c commit_size must be greater than 0 and less than or equal to @c
 * reserve_size.
 *
 * @return A pointer to the new arena.
 *
 * @see <a
 * href="https://man7.org/linux/man-pages/man2/mmap.2.html"
 * target="_blank">mmap()</a>, <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc"
 * target="_blank">VirtualAlloc()</a>
 */
arena_t *arena_new(u64 reserve_size, u64 commit_size);

/**
 * @brief Free an \ref arena.
 *
 * @details Frees an \ref arena using <a
 * href="https://man7.org/linux/man-pages/man3/munmap.3p.html"
 * target="_blank">munmap()</a> or <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree"
 * target="_blank">VirtualFree()</a>, depending on system, this is rarely needed
 * cause the kernel usually frees these pages for you, but its useful if you use
 * multiple arenas..
 *
 * @param arena The \ref arena to free.
 *
 * @pre @c arena must be valid and cannot be `null`.
 *
 * @see <a href="https://man7.org/linux/man-pages/man3/munmap.3p.html"
 * target="_blank">munmap()</a>, <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree"
 * target="_blank">VirtualFree()</a>
 */
void arena_free(arena_t *arena);

//
//
//

/**
 * @brief Allocate a chunk of memory to the \ref arena.
 *
 * @note This function is not meant to be run directly, and is called by the
 * \ref arena_alloc() macro.
 *
 * @details Through basically pushing the position forwards and returning a
 * small chunk of memory from the committed heap.
 *
 * @param arena The \ref arena to allocate from.
 * @param size The size of the chunk to allocate.
 *
 * @pre
 * - @c arena and @c size must be valid and cannot be `null`.
 * - @c size must be greater than 0.
 *
 * @return A pointer to the allocated chunk.
 *
 * @see \ref arena_alloc()
 */
void *__arena_alloc(struct arena *arena, u64 size);

/**
 * @brief Allocate a chunk of zeroed memory to the \ref arena.
 *
 * @note This function is not meant to be run directly, and is called by the
 * \ref arena_alloc_zeroed() macro.
 *
 * @details Through basically pushing the position forwards and returning a
 * small chunk of memory from the committed heap.
 *
 * @param arena The \ref arena to allocate from.
 * @param size The size of the chunk to allocate.
 *
 * @pre
 * - @c arena and @c size must be valid and cannot be `null`.
 * - @c size must be greater than 0.
 *
 * @return A pointer to the allocated chunk.
 *
 * @see \ref arena_alloc()
 */
void *__arena_alloc_zeroed(struct arena *arena, u64 size);

/**
 * @brief Deallocate a chunk of memory from the \ref arena.
 *
 * @note This function is not meant to be run directly, and is called by the
 * \ref arena_dealloc() macro.
 *
 * @details Through basically popping back the position, so that it may
 * be reused for other allocations.
 *
 * @param arena The \ref arena to deallocate from.
 * @param size The size of the chunk to deallocate.
 *
 * @pre
 * - @c arena and @c size must be valid and cannot be `null`.
 * - @c size must be greater than 0.
 *
 * @see \ref arena_dealloc()
 */
void __arena_dealloc(struct arena *arena, u64 size);

//
//
//

/**
 * @brief Allocate a chunk of memory from the \ref arena.
 *
 * @details Through pushing the commit position of \ref arena, and returning
 * the allocated chunk that you specify, the reason for it being a macro is due
 * to being able to specify the type, this will automatically grow the commit
 * size if it's too small, all this logic resides in \ref __arena_alloc().
 *
 * @param arena The \ref arena to allocate from.
 * @param type The type of the chunk to allocate.
 * @param size The size of the chunk to allocate.
 *
 * @pre
 * - @c arena, @c type, and @c size must be valid and cannot be `null`.
 * - @c type must be a valid type.
 *
 * @return A pointer to the allocated chunk.
 *
 * @see \ref __arena_alloc()
 */
#define arena_alloc(arena, type, size)                                         \
  __arena_alloc(arena, sizeof(type) * size);

/**
 * @brief Allocate a chunk of zeroed memory from the \ref arena.
 *
 * @details Through pushing the commit position of \ref arena, and returning
 * the allocated chunk that you specify, the reason for it being a macro is due
 * to being able to specify the type, this will automatically grow the commit
 * size if it's too small, all this logic resides in \ref
 * __arena_alloc_zeroed().
 *
 * @param arena The \ref arena to allocate from.
 * @param type The type of the chunk to allocate.
 * @param size The size of the chunk to allocate.
 *
 * @pre
 * - @c arena, @c type, and @c size must be valid and cannot be `null`.
 * - @c type must be a valid type.
 *
 * @return A pointer to the allocated chunk.
 *
 * @see \ref __arena_alloc_zeroed()
 */
#define arena_alloc_zeroed(arena, type, size)                                  \
  __arena_alloc_zeroed(arena, sizeof(type) * size);

/**
 * @brief Deallocate a chunk of memory from the \ref arena.
 *
 * @details Through popping the position from the \ref arena, using
 * \ref __arena_dealloc(), the reason this is a macro is to be able to pass
 * type.
 *
 * @param arena The \ref arena to deallocate from.
 * @param type The type of the chunk to deallocate.
 * @param size The size of the chunk to deallocate.
 *
 * @pre
 * - @c arena, @c type, and @c size must be valid and cannot be `null`.
 * - @c type must be a valid type.
 *
 * @see \ref __arena_dealloc()
 */
#define arena_dealloc(arena, type, size)                                       \
  __arena_dealloc(arena, sizeof(type) * size);

//
//
//

/**
 * @brief Set @c arena position to @pos.
 *
 * @details Through setting the position of the \ref arena to @c pos, useful for
 * clearing or removing a chunk of data you know you dont need any more. Uses
 * \ref __arena_dealloc() internally.
 *
 * @param arena The \ref arena to deallocate from.
 * @param pos The position to deallocate to.
 *
 * @pre
 * - @c arena and @c pos must be valid and cannot be `null`.
 * - @c pos must be greater than 0.
 *
 * @see \ref __arena_dealloc()
 */
void arena_dealloc_to(arena_t *arena, u64 pos);

/**
 * @brief Clear the \ref arena.
 *
 * @details Pops the \ref arena to the base position of `ARENA_BASE_POS` using
 * \ref arena_dealloc_to(), then flushes the commit position using
 * `mem_decommit()` and `mem_commit()`.
 *
 * @param arena The \ref arena to clear.
 *
 * @pre @c arena must be valid and cannot be `null`.
 *
 * @see \ref arena_dealloc_to()
 */
void arena_clear(arena_t *arena);

//
//
//

/**
 * @brief Create a new \ref temp_arena.
 *
 * @details Through saving a snapshot of the current `pos` param of the \ref
 * arena, to then pop back to when you run \ref temp_arena_free().
 *
 * @param arena The \ref arena to create the temporary arena from.
 *
 * @pre @c arena must be valid and cannot be `null`.
 *
 * ®return The new \ref temp_arena.
 *
 * @see \ref temp_arena_free()
 */
temp_arena_t temp_arena_new(arena_t *arena);

/**
 * @brief Free a \ref temp_arena.
 *
 * @details By popping the \ref arena back to the position it was at when the
 * temp
 * \ref arena was initialized using \ref temp_arena_new().
 *
 * @param temp The \ref temp_arena to free.
 *
 * @pre @c temp must be valid and cannot be `null`.
 *
 * @see \ref temp_arena_new()
 */
void temp_arena_free(temp_arena_t temp);

#endif // !HTILS_ARENA_H
