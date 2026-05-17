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

typedef struct arena arena_t;
struct arena {
  u64 reserved;
  u64 committed;
  u64 pos;
  u64 commit_pos;
};

typedef struct temp_arena temp_arena_t;
struct temp_arena {
  struct arena *arena;
  u64 start_pos;
};

//
//
//

/*
 * @brief Create a new arena.
 *
 * @details Initializes a new arena using VirtualAlloc or mmap, depending on
 * system.
 *
 * @param reserve_size The size of the arena to reserve.
 * @param commit_size The size of the arena to commit.
 *
 * @return A pointer to the new arena.
 */
arena_t *arena_new(u64 reserve_size, u64 commit_size);

/*
 * @brief Free an arena.
 *
 * @details Frees an arena using VirtualFree or munmap, depending on system,
 * this is rarely needed cause the kernel usually frees these pages for you, but
 * its useful if you use multiple arenas..
 *
 * @param arena The arena to free.
 */
void arena_free(arena_t *arena);

//
//
//

/*
 * @brief Allocate a chunk of memory to the arena.
 *
 * @note This function is not meant to be run directly, and is called by the
 * `arena_alloc` macro.
 *
 * @details This function basically pushes the position forwards and returns a
 * small chunk of memory from the committed heap.
 *
 * @param arena The arena to allocate from.
 * @param size The size of the chunk to allocate.
 *
 * @return A pointer to the allocated chunk.
 */
void *__arena_alloc(struct arena *arena, u64 size);

/*
 * @brief Deallocate a chunk of memory from the arena.
 *
 * @note This function is not meant to be run directly, and is called by the
 * `arena_dealloc` macro.
 *
 * @details This function basically just pops back the position, so that it may
 * be reused for other allocations.
 *
 * @param arena The arena to deallocate from.
 * @param size The size of the chunk to deallocate.
 */
void __arena_dealloc(struct arena *arena, u64 size);

//
//
//

/*
 * @brief Allocate a chunk of memory from the arena.
 *
 * @details This macro allocates a chunk of memory from the arena, and returns
 * the allocated chunk that you specify, the reason for it being a macro is due
 * to being able to specify the type.
 *
 * @param arena The arena to allocate from.
 * @param type The type of the chunk to allocate.
 * @param size The size of the chunk to allocate.
 *
 * @return A pointer to the allocated chunk.
 */
#define arena_alloc(arena, type, size)                                         \
  __arena_alloc(arena, sizeof(type) * size);

/*
 * @brief Deallocate a chunk of memory from the arena.
 *
 * @details This macro deallocates a chunk of memory from the arena, using the
 * `__arena_dealloc` function, the reason this is a macro is to be able to pass
 * type.
 *
 * @param arena The arena to deallocate from.
 * @param type The type of the chunk to deallocate.
 * @param size The size of the chunk to deallocate.
 */
#define arena_dealloc(arena, type, size)                                       \
  __arena_dealloc(arena, sizeof(type) * size);

//
//
//

/*
 * @brief Deallocate in the arena to a select position within it.
 *
 * @details This function is used to deallocate a chunk of memory from the arena
 * to a specific position within it, useful for clearing or removing a chunk of
 * data you know you dont need any more.
 *
 * @param arena The arena to deallocate from.
 * @param pos The position to deallocate to.
 */
void arena_dealloc_to(arena_t *arena, u64 pos);

/*
 * @brief Clear the arena.
 *
 * @details This function is used to clear the arena, it will deallocate/pop to
 * the initial position of the arena.
 *
 * @param arena The arena to clear.
 */
void arena_clear(arena_t *arena);

//
//
//

/*
 * @brief Create a new temporary arena.
 *
 * @details This function creates a new temporary arena, for temporary
 * processing, whether it's like making a buffer for something to then copy to a
 * more permanent buffer.
 *
 * @param arena The arena to create the temporary arena from.
 *
 * ®return The new temporary arena.
 */
temp_arena_t temp_arena_new(arena_t *arena);

/*
 * @brief Free a temporary arena.
 *
 * @details This function frees a temporary arena, by just popping to the
 * start_position field.
 *
 * @param temp The temporary arena to free.
 */
void temp_arena_free(temp_arena_t temp);

#endif // !HTILS_ARENA_H
