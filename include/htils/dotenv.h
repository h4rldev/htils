#ifndef HTILS_DOTENV_H
#define HTILS_DOTENV_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

//
//
//

/**
 * @brief Load a dotenv file, or a path with a .env file within it.
 *
 * @param path The path where a .env file is located or the explicit relative or
 * complete path to the .env file
 * @param arena The arena to allocate from.
 *
 * @pre
 * - @c path, and @c arena must be valid, and not null.
 * - @c path must be a valid path.
 *
 * @return amount of variables loaded, -1 on failure.
 */
i32 htils_dotenv_load(arena_t *arena, const string *path);

#endif // !HTILS_DOTENV_H
