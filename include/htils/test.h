#ifndef HTILS_TEST_H
#define HTILS_TEST_H

#include <stdlib.h>

//
//
//

#include <htils/arena.h>
#include <htils/basictypes.h>

#define COLOR_GREEN "\x1b[32m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

//
//
//

/** Test function type. */
typedef const cstr *test_fn(arena_t *arena);

//
//
//

/** The test assertion, basically signals if condition is false, the test fails.
 */
#define HTILS_TEST_ASSERT(cond, msg)                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "   %sFAIL%s: %s:%d: %s\n", COLOR_RED, COLOR_RESET,      \
              __FILE__, __LINE__, msg);                                        \
      return (msg);                                                            \
    }                                                                          \
  } while (0)

/** The test macro for test declarations. */
#define HTILS_TEST(name) static const cstr *htils_test_##name(arena_t *arena)

/** The runtime for the tests, uses temporary arenas. */
#define HTILS_TEST_RUN(name, failures)                                         \
  do {                                                                         \
    if (arena == null) {                                                       \
      fprintf(stderr, "No arena found, did you forget to initialize one?\n");  \
      exit(1);                                                                 \
    }                                                                          \
                                                                               \
    temp_arena_t temp_arena = temp_arena_new(arena);                           \
    fprintf(stderr, "Running test: %s...\n", #name);                           \
    const cstr *result = htils_test_##name(temp_arena.arena);                  \
    temp_arena_free(temp_arena);                                               \
                                                                               \
    if (!result)                                                               \
      fprintf(stderr, "   %sPASS%s: %s\n", COLOR_GREEN, COLOR_RESET, #name);   \
    else                                                                       \
      (failures)++;                                                            \
  } while (0)

#define HTILS_TEST_PASS null

#endif // !HTILS_TEST_H
