#include <stddef.h>
#include <string.h>

//
//
//

#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/basictypes.h>

//
//
//

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

#define ALIGN_POW2(num, pow)                                                   \
  (((u64)(num) + ((u64)(pow) - 1)) & (~((u64)(pow) - 1)))

#define ARENA_BASE_POS (sizeof(arena_t))
#define ARENA_ALIGNMENT (_Alignof(max_align_t))

//
//
//

#if defined(__linux__)

//
//
//

#include <stdbool.h>
#include <sys/mman.h>
#include <unistd.h>

//
//
//

/*
 * @brief Gets the page size of the system.
 *
 * @return The page size of the system.
 */
static u32 get_page_size(void) { return (u32)sysconf(_SC_PAGESIZE); }

/*
 * @brief Reserves a chunk of memory from the system.
 *
 * @details This function reserves a private and anonymous memory mapping, with
 * no protections.
 *
 * @param size The size of the chunk to reserve.
 *
 * @return A pointer to the reserved memory
 */
static void *mem_reserve(u64 size) {
  htils_assert(size > 0 && "Size must be greater than 0.");

  void *ret = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  htils_assert(ret != MAP_FAILED && "Failed to reserve memory.");
  return ret;
}

/*
 * @brief Releases a chunk of memory from the system.
 *
 * @details This function releases a memory mapping, with
 * munmap.
 *
 * @param ptr The pointer to the memory to release.
 * @param size The size of the memory to release.
 *
 * @return True if the memory was released, false if it failed.
 */
static bool mem_release(void *ptr, u64 size) {
  htils_assert(ptr != null && "Pointer cannot be null.");
  htils_assert(size > 0 && "Size must be greater than 0.");

  return (i32)munmap(ptr, size) == 0;
}

/*
 * @brief Commits a chunk of memory to the system.
 *
 * @details This function commits a memory mapping, with
 * READ and WRITE premissions, basically making a memory map usable.
 *
 * @param ptr The pointer to the memory to commit.
 * @param size The size of the memory to commit.
 *
 * @return True if the memory was committed, false if it failed.
 */
static bool mem_commit(void *ptr, u64 size) {
  htils_assert(ptr != null && "Pointer cannot be null.");
  htils_assert(size > 0 && "Size must be greater than 0.");
  return (i32)mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
}

/*
 * @brief Decommits a chunk of memory from the system.
 *
 * @details This function decommits a memory mapping, by
 * first removing the protections, and then advising that it's unneeded heap.
 *
 * @param ptr The pointer to the memory to decommit.
 * @param size The size of the memory to decommit.
 *
 * @return True if the memory was decommitted, false if it failed.
 */
static bool mem_decommit(void *ptr, u64 size) {
  htils_assert(ptr != null && "Pointer cannot be null.");
  htils_assert(size > 0 && "Size must be greater than 0.");

  i32 ret = mprotect(ptr, size, PROT_NONE);
  htils_assert(ret == 0 && "Failed to decommit memory, mprotect failed.");
  return (i32)madvise(ptr, size, MADV_DONTNEED) == 0;
}

//
//
//

#elif defined(_WIN32)

//
//
//

#include <stdbool.h>
#include <windows.h>

//
//
//

/*
 * @brief Gets the page size of the system.
 *
 * @details Uses GetSystemInfo to retrieve system info, including the page size,
 * then returns the dwPageSize field.
 *
 * @return The page size of the system.
 */
static u32 get_page_size(void) {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  return (u32)sys_info.dwPageSize;
}

/*
 * @brief Reserves a chunk of memory from the system.
 *
 * @details This function reserves virtual address space, with
 * VirtualAlloc.
 *
 * @param size The size of the chunk to reserve.
 *
 * @return A pointer to the reserved memory.
 */
static void *mem_reserve(u64 size) {
  htils_assert(size > 0 && "Size must be greater than 0.");

  void *ret = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
  htils_assert(ret != NULL && "Failed to reserve memory, VirtualAlloc failed.");
  return ret;
}

/*
 * @brief Releases a chunk of memory from the system.
 *
 * @details This function releases the virtual address space,
 * with VirtualFree.
 *
 * @param ptr The pointer to the memory to release.
 * @param size The size of the memory to release.
 *
 * @note size is unused in Windows, but we keep it in to keep the api the same
 * across systems.
 *
 * @return True if the memory was released, false if it failed.
 */
static bool mem_release(void *ptr, u64 size) {
  htils_assert(ptr != null && "Pointer cannot be null.");

  (void)size;
  return (i32)VirtualFree(ptr, 0, MEM_RELEASE) != 0;
}

/*
 * @brief Commits a chunk of memory to the system.
 *
 * @details This function commits the virtual address space, with
 * VirtualAlloc.
 *
 * @param ptr The pointer to the memory to commit.
 * @param size The size of the memory to commit.
 *
 * @return True if the memory was committed, false if it failed.
 */
static bool mem_commit(void *ptr, u64 size) {
  htils_assert(ptr != null && "Pointer cannot be null.");
  htils_assert(size > 0 && "Size must be greater than 0.");

  void *ret = VirtualAlloc(ptr, (SIZE_T)size, MEM_COMMIT, PAGE_READWRITE);
  return ret != NULL;
}

/*
 * @brief Decommits a chunk of memory from the system.
 *
 * @details This function decommits the virtual address space, with VirtualFree.
 *
 * @param ptr The pointer to the memory to decommit.
 * @param size The size of the memory to decommit.
 *
 * @return True if the memory was decommitted, false if it failed.
 */
static bool mem_decommit(void *ptr, u64 size) {
  htils_assert(ptr != null && "Pointer cannot be null.");
  htils_assert(size > 0 && "Size must be greater than 0.");

  return (i32)VirtualFree(ptr, 0, MEM_DECOMMIT) != 0;
}

//
//
//

#else
#error "Unsupported platform."
#endif

//
//
//
arena_t *arena_new(u64 reserve_size, u64 commit_size) {
  u32 page_size = get_page_size();

  htils_assert(page_size > 0 && "Failed to get page size.");
  htils_assert(reserve_size > 0 && "Reserve size must be greater than 0.");
  htils_assert(commit_size > 0 && "Commit size must be greater than 0.");
  htils_assert(reserve_size > commit_size &&
               "Commit size cannot be greater than reserve size.");

  reserve_size = ALIGN_POW2(reserve_size, page_size);
  commit_size = ALIGN_POW2(commit_size, page_size);

  struct arena *arena = mem_reserve(ARENA_BASE_POS + reserve_size);
  htils_assert(mem_commit(arena, commit_size) && "Failed to commit memory.");

  arena->reserved = reserve_size;
  arena->committed = commit_size;
  arena->pos = ARENA_BASE_POS;
  arena->commit_pos = commit_size;

  return arena;
}

void arena_free(arena_t *arena) {
  htils_assert(arena != NULL && "Arena cannot be null.");
  htils_assert(mem_release(arena, arena->reserved) &&
               "Failed to release arena.");
}

//
//
//

void *__arena_alloc(struct arena *arena, u64 size) {
  htils_assert(arena != null && "Arena cannot be null.");
  htils_assert(size > 0 && "Size must be greater than 0.");

  u64 pos_aligned = ALIGN_POW2(arena->pos, ARENA_ALIGNMENT);
  u64 new_pos = pos_aligned + size;

  htils_assert(new_pos < arena->reserved && "Arena is full.");

  if (new_pos > arena->commit_pos) {
    u64 new_commit_pos = new_pos;
    new_commit_pos += arena->committed - 1;
    new_commit_pos -= new_commit_pos % arena->committed;
    new_commit_pos = MIN(new_commit_pos, arena->reserved);

    u8 *mem = (u8 *)arena + arena->commit_pos;
    u64 commit_size = new_commit_pos - arena->commit_pos;

    htils_assert(mem_commit(mem, commit_size) &&
                 "Failed to commit new memory.");

    arena->commit_pos = new_commit_pos;
  }

  arena->pos = new_pos;
  u8 *out = (u8 *)arena + pos_aligned;
  memset(out, 0, size);

  return (void *)out;
}

void __arena_dealloc(struct arena *arena, u64 size) {
  htils_assert(arena != null && "Arena cannot be null.");

  size = MIN(size, arena->pos - ARENA_BASE_POS);
  arena->pos -= size;
}

//
//
//

void arena_dealloc_to(arena_t *arena, u64 pos) {
  htils_assert(arena != null && "Arena cannot be null.");

  u64 size = pos < arena->pos ? arena->pos - pos : 0;
  __arena_dealloc(arena, size);
}

void arena_clear(arena_t *arena) {
  htils_assert(arena != null && "Arena cannot be null.");
  arena_dealloc_to(arena, ARENA_BASE_POS);

  htils_assert(mem_decommit(arena, arena->commit_pos) &&
               "Failed to decommit memory.");
  htils_assert(mem_commit(arena, arena->committed) &&
               "Failed to recommit memory.");
  arena->commit_pos = arena->committed;
}

//
//
//

temp_arena_t temp_arena_new(arena_t *arena) {
  htils_assert(arena != null && "Arena cannot be null.");
  return (temp_arena_t){.arena = arena, .start_pos = arena->pos};
}

void temp_arena_free(temp_arena_t temp) {
  htils_assert(temp.arena != null && "Arena cannot be null.");
  arena_dealloc_to(temp.arena, temp.start_pos);
}

/// :3
