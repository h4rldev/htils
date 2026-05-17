#ifndef HTILS_FILE_H
#define HTILS_FILE_H

#include <stdio.h>

///
// Our headers
//

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

///
// Get the size of a file
//

/*
 * @brief Gets the size of a file from a file stream.
 *
 * @details Gets the size of a file, by opening the file and getting the size
 * from it.
 *
 * @param stream The file stream to get the size of.
 *
 * @return The size of the file.
 */
u64 file_size_stream(FILE *stream);

/*
 * @brief Gets the size of a file.
 *
 * @details Gets the size of a file, by opening the file and getting the size
 * from it.
 *
 * @param path The path to the file to get the size of.
 *
 * @return The size of the file.
 */
u64 file_size(const string *path);

///
// Read from file
//

/*
 * @brief Reads the contents of a file from a file stream into a string.
 *
 * @details Reads the entire file 'til EOF and stores it in a string, with the
 * file content length in the len param.
 *
 * @param arena For allocating the contents of the file.
 * @param stream The file stream to read from.
 *
 * @return A string containing the contents of the file.
 */
string *read_file_from_stream(arena_t *arena, FILE *stream);

/*
 * @brief Reads the contents of a file into a string.
 *
 * @details Reads the entire file 'til EOF and stores it in a string, with the
 * file content length being stored in the len param of the string.
 *
 * @param arena For allocating the contents of the file.
 * @param path The path to the file to read.
 *
 * @return A string containing the contents of the file.
 */
string *read_file(arena_t *arena, const string *path);

///
// Read from file by bytes
//

/*
 * @brief Reads bytes bytes from a file stream into a string.
 *
 * @details Reads bytes bytes from a file stream into a string, by allocating
 * a block that is bytes big and reading bytes bytes into that block.
 *
 * @param arena For allocating the contents of the file.
 * @param stream The file stream to read from.
 * @param bytes The number of bytes to read.
 *
 * @return A string containing the contents of the file.
 */
string *read_file_from_stream_bytes(arena_t *arena, FILE *stream,
                                    const u64 bytes);

/*
 * @brief Reads bytes bytes from a file into a string.
 *
 * @details Reads bytes bytes from a file into a string, by allocating a block
 * that is bytes big and then reading bytes bytes into that block.
 *
 * @param arena For allocating the contents of the file.
 * @param path The path to the file to read.
 * @param bytes The number of bytes to read.
 *
 * @return A string containing the contents of the file
 */
string *read_file_bytes(arena_t *arena, const string *path, const u64 bytes);

///
// Write to file
//

/*
 * @brief Writes a string to a file stream.
 *
 * @details Writes a string to a file stream, by writing len characters to
 * that file thats associated with that file stream.
 *
 * @param stream The file stream to write to.
 * @param contents The string to write to the file.
 *
 * @return The number of bytes written.
 */
u64 write_to_file_stream(FILE *stream, const string *contents);

/*
 * @breif Writes a string to a file.
 *
 * @details Writes a string to a file, by writing len characters to that file.
 *
 * @param path The path to the file to write to.
 * @param contents The string to write to the file.
 *
 * @return The number of bytes written.
 */
u64 write_to_file(const string *path, const string *contents);

///
// Write to file by bytes
//

/*
 * @brief Writes bytes bytes from a string to a file stream.
 *
 * @param stream The file stream to write to.
 * @param contents The string to write to the file.
 *
 * @return The number of bytes written.
 */
u64 write_to_file_stream_bytes(FILE *stream, const string *contents,
                               const u64 bytes);

/*
 * @brief Writes bytes bytes from a string to a file.
 *
 * @param path The path to the file to write to.
 * @param contents The string to write to the file.
 *
 * @return The number of bytes written.
 */
u64 write_to_file_bytes(const string *path, const string *contents,
                        const u64 bytes);

///
// File IO misc. utilities
//

/*
 * @brief Makes a directory.
 *
 * @param path The path to the directory to make.
 *
 * @return True if the directory was made, false if it failed.
 */
b32 make_dir(const string *path);

/*
 * @brief Checks if a path exists.
 *
 * @param path The path to check.
 *
 * @return True if the path exists, false if it doesn't.
 */
b32 does_path_exist(const string *path);

#endif // !HTILS_FILE_H
