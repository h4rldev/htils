#ifndef HTILS_ASSERT_H
#define HTILS_ASSERT_H

//
//
//

/* For using the gnu extension program_invocation_short_name */
#define _GNU_SOURCE

//
//
//

#include <htils/basictypes.h>

//
//
//

/*
 * @brief The name of the program, exposed by the GNU standard.
 *
 * @details This variable is exposed by the GNU standard, and is used to get the
 * program name without relying on argv[0].
 *
 * @note This variable depends on GNU extensions, and is not portable.
 */
extern cstr *program_invocation_short_name;

//
//
//

/*
 * @brief Terminates the program with an error message.
 *
 * @details This function terminates the program with an error message, this is
 * used instead of normal assert cause assert uses abort.
 *
 * @note This function should never be run directly, and is run by
 * `htils_assert` on a failed assertion.
 *
 * @param expr_str The expression as string.
 * @param executable The executable name.
 * @param file The file name.
 * @param line The line number.
 * @param func The function name.
 */
_Noreturn void __htils_assert_fail(const cstr *expr_str, const cstr *executable,
                                   const cstr *file, u32 line,
                                   const cstr *func);

#define htils_assert(expr)                                                     \
  ((expr) ? (void)0                                                            \
          : __htils_assert_fail(#expr, program_invocation_short_name,          \
                                __FILE__, __LINE__, __func__))

#endif // !DAPPIE_ASSERT_H
