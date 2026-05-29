#ifndef H2OTILS_TEST_H
#define H2OTILS_TEST_H

#include <h2o/memory.h>

//
//
//

#include <htils/basictypes.h>

#define COLOR_GREEN "\x1b[32m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

//
//
//

/** The test assertion, basically signals if condition is false, the test fails.
 */
#define H2OTILS_TEST_ASSERT(cond, msg)                                         \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "   %sFAIL%s: %s:%d: %s\n", COLOR_RED, COLOR_RESET,      \
              __FILE__, __LINE__, msg);                                        \
      return (msg);                                                            \
    }                                                                          \
  } while (0)

/** The test macro for test declarations. */
#define H2OTILS_TEST(name)                                                     \
  static const cstr *h2otils_test_##name(h2o_mem_pool_t *pool)

/** The runtime for the tests, uses temporary arenas. */
#define H2OTILS_TEST_RUN(name)                                                 \
  do {                                                                         \
    test_count++;                                                              \
                                                                               \
    h2o_mem_pool_t pool;                                                       \
    h2o_mem_init_pool(&pool);                                                  \
    fprintf(stderr, "Running test: %s...\n", #name);                           \
    const cstr *result = h2otils_test_##name(&pool);                           \
    h2o_mem_clear_pool(&pool);                                                 \
    if (!result)                                                               \
      fprintf(stderr, "   %sPASS%s: %s\n", COLOR_GREEN, COLOR_RESET, #name);   \
    else if (result[0] == '@') {                                               \
      fprintf(stderr, "   %sSKIPPED%s: %s\n", COLOR_CYAN, COLOR_RESET, #name); \
      skips++;                                                                 \
    } else                                                                     \
      (failures)++;                                                            \
  } while (0)

/** Handle the test results, prints the results and returns 0 if all tests pass,
 * returns 1 if any test fails. */
#define H2OTILS_TEST_RESULT()                                                  \
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

/** Lil macro that makes test passes more opaque */
#define H2OTILS_TEST_PASS null

/** Lil macro to tell the test suite to skip a test */
#define H2OTILS_TEST_SKIP "@"

#endif // !H2OTILS_TEST_H
