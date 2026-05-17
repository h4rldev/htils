#ifndef HTILS_PATH_H
#define HTILS_PATH_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

///
//
//

/*
 * @brief Gets the absolute path of a path.
 *
 * @details Gets the absolute path of a path, will only work on a partial yet
 * relative path.
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the absolute path of.
 *
 * @return The absolute path.
 */
string *path_canonical(arena_t *arena, const string *path);

/*
 * @brief Get the basename i.e the filename+extension / final dir of a path.
 *
 * @details Get the basename i.e the filename+extension of a path, by trimming
 * previous /
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the basename of.
 *
 * @return The basename of the path.
 */
string *path_basename(arena_t *arena, const string *path);

/*
 * @brief Get the dirname of a path.
 *
 * @details Get the dirname of a path, trimming away the file from the path.
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the dirname of.
 *
 * @return The dirname of the path
 */
string *path_dirname(arena_t *arena, const string *path);

/*
 * @brief Get the extension of a path.
 *
 * @details Gets the last extension of a path, by splitting the path by '.' and
 * getting the last of that in the array.
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the extension of.
 *
 * @return The extension of the path, if the path is a dir, return null.
 */
string *path_extension(arena_t *arena, const string *path);

/*
 * @brief Get the stem of a path.
 *
 * @details Gets the stem of a path, i.e the extension prior to the extension.
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the stem of.
 *
 * @return The stem of the path
 */
string *path_stem(arena_t *arena, const string *path);

//
//
//

/*
 * @brief Joins two paths together.
 *
 * @details Joins two paths together, bascially a wrapper + extension of
 * string_concat.
 *
 * @param arena The arena to allocate from.
 * @param first The first path to join.
 * @param second The second path to join.
 *
 * @return The joined path.
 */
string *path_join(arena_t *arena, const string *first, const string *second);

#endif // !HTILS_PATH_H
