#ifndef HTILS_TEST_H
#define HTILS_TEST_H

/***********************************/

#include <htils/arena.h>
#include <htils/basictypes.h>

/***********************************/

/** @internal */
#define COLOR_GREEN "\x1b[32m"

/** @internal */
#define COLOR_CYAN "\x1b[36m"

/** @internal */
#define COLOR_RED "\x1b[31m"

/** @internal */
#define COLOR_RESET "\x1b[0m"

/** @brief Fail the test with @c msg when @c cond is false. */
#define HTILS_TEST_ASSERT(cond, msg)                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "   %sFAIL%s: %s:%d: %s\n", COLOR_RED, COLOR_RESET,      \
              __FILE__, __LINE__, msg);                                        \
      return (msg);                                                            \
    }                                                                          \
  } while (0)

//
//
//

/**
 * @brief Declare a test.
 *
 * @details Expands to a `static` function `htils_test_<name>` taking an
 * \ref arena and returning its result (\ref HTILS_TEST_PASS, or a failure
 * message).
 */
#define HTILS_TEST(name) static const cstr *htils_test_##name(arena_t *arena)

//
//
//

/**
 * @brief Run a declared test.
 *
 * @details Runs @c name in a fresh temporary arena, printing PASS, FAIL, or
 * SKIPPED. Expects the surrounding runner to have `arena`, `test_count`,
 * `skips`, and `failures` in scope, and is meant to be paired with
 * \ref HTILS_TEST_RESULT().
 */
#define HTILS_TEST_RUN(name)                                                   \
  do {                                                                         \
    test_count++;                                                              \
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
    else if (result[0] == '@') {                                               \
      fprintf(stderr, "   %sSKIPPED%s: %s\n", COLOR_CYAN, COLOR_RESET, #name); \
      skips++;                                                                 \
    } else                                                                     \
      (failures)++;                                                            \
  } while (0)

//
//
//

/** Handle the test results, prints the results and returns 0 if all tests pass,
 * returns 1 if any test fails. */
#define HTILS_TEST_RESULT()                                                    \
  do {                                                                         \
    if (failures > 0) {                                                        \
      fprintf(stderr, "\n%u Tests %sFAILED%s.\n", failures, COLOR_RED,         \
              COLOR_RESET);                                                    \
      fprintf(stderr, "%sSKIPPED%s %u tests.\n", COLOR_CYAN, COLOR_RESET,      \
              skips);                                                          \
      return 1;                                                                \
    }                                                                          \
                                                                               \
    fprintf(stderr, "\nAll %u tests %sPASSED%s.\n", test_count, COLOR_GREEN,   \
            COLOR_RESET);                                                      \
    fprintf(stderr, "%sSKIPPED%s %u tests.\n", COLOR_CYAN, COLOR_RESET,        \
            skips);                                                            \
    return 0;                                                                  \
  } while (0)

//
//
//

/** @brief Return value for a passing test. */
#define HTILS_TEST_PASS null

//
//
//

/** @brief Return this from a test to mark it skipped. */
#define HTILS_TEST_SKIP "@"

#endif // !HTILS_TEST_H
