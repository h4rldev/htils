#ifndef HTILS_ARENA_H
#define HTILS_ARENA_H

/***********************************/

#ifdef HTILS_THREAD_SAFE
#include <htils/atomic_types.h>
#include <stdatomic.h>
#include <threads.h>
#endif

#include <htils/basictypes.h>

/***********************************/

#define KiB(bytes) ((u64)bytes << 10)
#define MiB(bytes) ((u64)bytes << 20)
#define GiB(bytes) ((u64)bytes << 30)

#ifndef HTILS_THREAD_SAFE
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
#else
/**
 * @brief An atomic arena.
 *
 * @param reserved The size of the Atomic arena.
 * @param committed The size of the committed heap.
 * @param pos The current atomic position of the heap.
 * @param commit_pos The current committed position of the heap.
 * @param commit_mtx The mutex for the committed position.
 */
typedef struct arena {
  u64 reserved;
  u64 committed;
  atomic_u64 pos;
  u64 commit_pos;
  mtx_t commit_mtx;
} arena_t;
#endif

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
 * @details Reserves `reserve_size` bytes of virtual address space (plus the
 * \ref arena header) and commits the first `commit_size` bytes. Later
 * allocations commit further `commit_size` chunks as needed, up to the reserve.
 * Backed by <a
 * href="https://man7.org/linux/man-pages/man2/mmap.2.html"
 * target="_blank">mmap()</a> or <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc"
 * target="_blank">VirtualAlloc()</a>, depending on system.
 *
 * @pre
 * - @c reserve_size must be greater than 0.
 * - @c commit_size must be greater than 0 and less than @c reserve_size.
 *
 * @param reserve_size The size of the \ref arena to reserve.
 * @param commit_size The size of the \ref arena to commit.
 *
 * @return A pointer to the new arena.
 *
 * @see <a
 * href="https://man7.org/linux/man-pages/man2/mmap.2.html"
 * target="_blank">mmap()</a>, <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc"
 * target="_blank">VirtualAlloc()</a>, \ref arena_free()
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
 * @pre @c arena must be valid and cannot be `null`.
 *
 * @param arena The \ref arena to free.
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
 * @details Advances the position and returns a `size`-byte chunk from the
 * committed region, committing another chunk first if the request would cross
 * it.
 *
 * @pre
 * - @c arena must be valid and cannot be `null`.
 * - @c size must be greater than 0.
 *
 * @param arena The \ref arena to allocate from.
 * @param size The size of the chunk to allocate.
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
 * @details Like \ref __arena_alloc(), with the returned chunk zeroed.
 *
 * @pre
 * - @c arena must be valid and cannot be `null`.
 * - @c size must be greater than 0.
 *
 * @param arena The \ref arena to allocate from.
 * @param size The size of the chunk to allocate.
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
 * @details Moves the position back by @c size, so the freed tail can be reused
 * by later allocations.
 *
 * @pre
 * - @c arena must be valid and cannot be `null`.
 * - @c size must be greater than 0.
 *
 * @param arena The \ref arena to deallocate from.
 * @param size The size of the chunk to deallocate.
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
 * @pre
 * - @c arena must be valid and cannot be `null`.
 * - @c type must be a valid type and @c size greater than 0.
 *
 * @param arena The \ref arena to allocate from.
 * @param type The type of the chunk to allocate.
 * @param size The size of the chunk to allocate.
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
 * @pre
 * - @c arena must be valid and cannot be `null`.
 * - @c type must be a valid type and @c size greater than 0.
 *
 * @param arena The \ref arena to allocate from.
 * @param type The type of the chunk to allocate.
 * @param size The size of the chunk to allocate.
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
 * @pre
 * - @c arena must be valid and cannot be `null`.
 * - @c type must be a valid type and @c size greater than 0.
 *
 * @param arena The \ref arena to deallocate from.
 * @param type The type of the chunk to deallocate.
 * @param size The size of the chunk to deallocate.
 *
 * @see \ref __arena_dealloc()
 */
#define arena_dealloc(arena, type, size)                                       \
  __arena_dealloc(arena, sizeof(type) * size);

//
//
//

/**
 * @brief Set @c arena position to @c pos.
 *
 * @details Moves the position back to `pos`, freeing everything allocated after
 * it. This is what \ref temp_arena_free() and \ref arena_clear() are built on.
 *
 * @pre
 * - @c arena and @c pos must be valid and cannot be `null`.
 * - @c pos must be greater than 0.
 *
 * @param arena The \ref arena to deallocate from.
 * @param pos The position to deallocate to.
 *
 * @see \ref __arena_dealloc()
 */
void arena_dealloc_to(arena_t *arena, u64 pos);

/**
 * @brief Clear the \ref arena.
 *
 * @details Resets the position to the base with \ref arena_dealloc_to(), then
 * decommits everything past the initial commit, returning it to the OS. The
 * initial `commit_size` region stays committed.
 *
 * @pre @c arena must be valid and cannot be `null`.
 *
 * @param arena The \ref arena to clear.
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
 * @details Snapshots the arena's current position, to be restored by
 * \ref temp_arena_free().
 *
 * @pre @c arena must be valid and cannot be `null`.
 *
 * @param arena The \ref arena to create the temporary arena from.
 *
 * @return The new \ref temp_arena.
 *
 * @see \ref temp_arena_free()
 */
temp_arena_t temp_arena_new(arena_t *arena);

/**
 * @brief Free a \ref temp_arena.
 *
 * @details Moves the arena back to the position captured by
 * \ref temp_arena_new().
 *
 * @pre @c temp must be valid and cannot be `null`.
 *
 * @param temp The \ref temp_arena to free.
 *
 * @see \ref temp_arena_new()
 */
void temp_arena_free(temp_arena_t temp);

#endif // !HTILS_ARENA_H
