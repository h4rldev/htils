#ifndef HTILS_DOTENV_H
#define HTILS_DOTENV_H

/***********************************/

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

/***********************************/

/**
 * @brief Load environment variables from a `.env` file.
 *
 * @details Parses `KEY=VALUE` lines and sets each as a process environment
 * variable with `setenv()` (overwriting). @c path may be the `.env` file
 * itself, or a directory containing one, in which case the first `.env`
 * found is used.
 *
 * @param arena The arena to allocate from.
 * @param path The `.env` file, or a directory to search for one.
 *
 * @pre
 * - @c path and @c arena must be valid and not null.
 * - @c path must exist.
 *
 * @return The number of variables loaded, or -1 on failure.
 */
i32 htils_dotenv_load(arena_t *arena, const string *path);

#endif // !HTILS_DOTENV_H
