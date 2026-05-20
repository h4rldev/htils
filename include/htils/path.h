#ifndef HTILS_PATH_H
#define HTILS_PATH_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

///
//
//

/**
 * @brief Gets the absolute/canonical path of @c path.
 *
 * @details Using <a
 * href="https://www.man7.org/linux/man-pages/man3/realpath.3.html"
 * target="_blank">realpath()</a> and then allocating the new canonical path
 * using \ref arena_alloc().
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the absolute path of.
 *
 * @pre @c arena and @c path must be valid and cannot be `null`.
 *
 * @return The absolute path.
 *
 * @see <a href="https://www.man7.org/linux/man-pages/man3/realpath.3.html"
 * target="_blank">realpath()</a>, \ref arena_alloc()
 */
string *path_canonical(arena_t *arena, const string *path);

/**
 * @brief Get the basename i.e the filename.extension / final dir of @c path.
 *
 * @details By trimming previous / using pointer arithmetic and allocating the
 * new string using \ref arena_alloc().
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the basename of.
 *
 * @pre @c arena and @c path must be valid and cannot be `null`.
 *
 * @return The basename of the path.
 *
 * @see \ref arena_alloc().
 */
string *path_basename(arena_t *arena, const string *path);

/**
 * @brief Get the dirname of a path.
 *
 * @details By trimming away the last entry of the path, and allocating the new
 * string using \ref arena_alloc().
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the dirname of.
 *
 * @pre @c arena and @c path must be valid and cannot be `null`.
 *
 * @return The dirname of the path.
 *
 * @see \ref arena_alloc().
 */
string *path_dirname(arena_t *arena, const string *path);

/**
 * @brief Get the extension of @c path.
 *
 * @details Trims to the last extension of @c path, through pointer arithmetic,
 * and allocating the new string using \ref arena_alloc().
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the extension of.
 *
 * @pre @c arena and @c path must be valid and cannot be `null`.
 *
 * @return The extension of the path, if the path is a dir, return null.
 *
 * @see \ref arena_alloc().
 */
string *path_extension(arena_t *arena, const string *path);

/**
 * @brief Get the stem of @c path.
 *
 * @details Trims to the stem of @c path, i.e the extension prior to the
 * extension, through pointer arithmetic, and allocating the new string using
 * \ref arena_alloc().
 *
 * @param arena The arena to allocate from.
 * @param path The path to get the stem of.
 *
 * @pre @c arena and @c path must be valid and cannot be `null`.
 *
 * @return The stem of the path
 *
 * @see \ref arena_alloc().
 */
string *path_stem(arena_t *arena, const string *path);

//
//
//

/**
 * @brief Joins two paths together.
 *
 * @details Works with paths accounting for missing trailing and leading
 * slashes.
 *
 * @param arena The arena to allocate from.
 * @param first The first path to join.
 * @param second The second path to join.
 *
 * @pre @c arena, @c first, and @c second must be valid and cannot be `null`.
 *
 * @return The joined path.
 */
string *path_join(arena_t *arena, const string *first, const string *second);

//
//
//

/**
 * @brief Makes a directory.
 *
 * @details By either:
 *  - For Linux: Creating the directory at @c path using <a
 * href="https://www.man7.org/linux/man-pages/man2/mkdir.2.html"
 * target="_blank">mkdir()</a>.
 *  - For Windows: Creating the directory at @c path using <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createdirectory"
 * target="_blank">CreateDirectory()</a>.
 *
 * @pre @c path must be valid, and cannot be `null`.
 *
 * @param path The path to the directory to make.
 *
 * @return True if the directory was made, false if it failed.
 *
 * @see <a href="https://www.man7.org/linux/man-pages/man2/mkdir.2.html"
 * target="_blank">mkdir()</a>, <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createdirectory"
 * target="_blank">CreateDirectory()</a>
 */
b32 make_dir(const string *path);

/**
 * @brief Checks if a path exists.
 *
 * @details By either:
 *  - For Linux: Checking if the path at @c path exists using <a
 * href="https://www.man7.org/linux/man-pages/man2/stat.2.html">stat()</a>.
 *  - For Windows: Checking if the path at @c path exists using <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesa">GetFileAttributes()</a>.
 *
 * @pre @c path must be valid, and cannot be `null`.
 *
 * @param path The path to check.
 *
 * @return True if the path exists, false if it doesn't.
 *
 * @see <a
 * href="https://www.man7.org/linux/man-pages/man2/stat.2.html">stat()</a>, <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesa">GetFileAttributes()</a>
 */
b32 does_path_exist(const string *path);

/**
 * @brief Removes a file or directory at @c path.
 *
 * @details By either:
 * - For Linux: Removing the file or directory at @c path using <a
 * href="https://www.man7.org/linux/man-pages/man3/remove.3.html">remove()</a>.
 * - For Windows: Removing the file or directory at @c path using <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shfileoperationw"
 * target="_blank">ShFileOperationW()</a>.
 *
 * @pre @c path must be valid, and cannot be `null`.
 *
 * @param path The path to remove.
 *
 * @return True if the path was removed, false if it failed.
 *
 * @see <a
 * href="https://www.man7.org/linux/man-pages/man3/remove.3.html">remove()</a>,
 * <a
 * href="https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shfileoperationw"
 * target="_blank">ShFileOperationW()</a>
 */
b32 path_remove(const string *path);

#endif // !HTILS_PATH_H
