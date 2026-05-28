#ifndef HTILS_FILE_H
#define HTILS_FILE_H

#include <stdio.h>
#include <sys/stat.h>

//
//
//

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

/** Safe macros for `fstat()` and `fileno()` thats compatible across platforms.
 */
#if defined(_WIN32)
#include <windows.h>

#define fileno _fileno
#define stat _stat64
#define fstat _fstati64
#endif

//
//
//

/**
 * @brief Gets the size of a file from a file stream.
 *
 * @details By reading the size using `fstat()` by first converting the file
 * stream into a file descriptor with `fileno()`. and then getting the stat
 * structure's `st_size` field.
 *
 * `fstat()` and `fileno()` are aliases:
 * - For Linux: Using <a
 * href="https://linux.die.net/man/2/fstat">fstat()</a> and
 * <a href="https://linux.die.net/man/3/fileno">fileno()</a>.
 * - For Windows: Using <a
 * href="https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fstat-fstat32-fstat64-fstati64-fstat32i64-fstat64i32">_fstati64()</a>
 * and <a
 * href="https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fileno">_fileno()</a>.
 *
 * @pre @c stream must be valid, preferably open with "rb" and cannot be `null`.
 *
 * @param stream The file stream to get the size of.
 *
 * @return The size of the file.
 *
 * @see <a href="https://linux.die.net/man/2/fstat" target="_blank">fstat()</a>,
 * <a href="https://linux.die.net/man/3/fileno" target="_blank">fileno()</a>, <a
 * href="https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fstat-fstat32-fstat64-fstati64-fstat32i64-fstat64i32"
 * target="_blank">_fstati64()</a>, <a
 * href="https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fileno"
 * target="_blank">_fileno()</a>.
 */
u64 file_size_stream(FILE *stream);

/**
 * @brief Gets the size of a file at @c path.
 *
 * @details Through opening the file and then running \ref file_size_stream().
 *
 * @pre @c path must be valid and cannot be `null`.
 *
 * @param path The path to the file to get the size of.
 *
 * @return The size of the file.
 *
 * @see \ref file_size_stream()
 */
u64 file_size(const string *path);

//
//
//

/**
 * @brief Reads the contents of a file from @c stream into a string.
 *
 * @details Reads the entire file 'til EOF and stores it in a string, with the
 * file content length in the `len` param using <a
 * href="https://en.cppreference.com/w/c/io/fread" target="_blank">fread()</a>.
 *
 * @pre
 * - @c arena must be valid antd cannot be `null`.
 * - @c stream must be valid and cannot be `null` preferably open with "rb".
 *
 * @param arena For allocating the contents of the file.
 * @param stream The file stream to read from.
 *
 * @return A \ref string containing the contents of the file.
 *
 * @see <a href="https://en.cppreference.com/w/c/io/fread"
 * target="_blank">fread()</a>
 */
string *read_file_from_stream(arena_t *arena, FILE *stream);

/**
 * @brief Reads the contents of a file at @c path into a string.
 *
 * @details Opens the file, reads the entire file 'til EOF with
 * \ref read_file_from_stream(), then returns the result.
 *
 * @pre
 * - @c arena and @c path must be valid and cannot be `null`.
 *
 * @param arena For allocating the contents of the file.
 * @param path The path to the file to read.
 *
 * @return A \ref string containing the contents of the file.
 *
 * @see \ref read_file_from_stream()
 */
string *read_file(arena_t *arena, const string *path);

//
//
//

/**
 * @brief Read @c bytes from file stream to string.
 *
 * @details By allocating a block using the \ref arena that is @c bytes big
 * and reading @c bytes into that block using <a
 * href="https://en.cppreference.com/w/c/io/fread" target="_blank">fread()</a>.
 *
 * @pre
 * - @c arena, @c stream, and @c bytes must be valid and cannot be `null`.
 * - @c stream should be preferably open with "rb".
 *
 * @param arena For allocating the contents of the file.
 * @param stream The file stream to read from.
 * @param bytes The number of bytes to read.
 *
 * @return A \ref string containing the contents of the file.
 *
 * @see <a href="https://en.cppreference.com/w/c/io/fread"
 * target="_blank">fread()</a>
 */
string *read_file_from_stream_bytes(arena_t *arena, FILE *stream,
                                    const u64 bytes);

/**
 * @brief Read @c bytes from a file path to string.
 *
 * @details By opening the file, reading @c bytes into a block using
 * \ref read_file_from_stream_bytes(), then returning the result.
 *
 * @pre @c arena, @c path, and @c bytes must be valid and cannot be `null`.
 *
 * @param arena For allocating the contents of the file.
 * @param path The path to the file to read.
 * @param bytes The number of bytes to read.
 *
 * @return A \ref string containing the contents of the file.
 *
 * @see \ref read_file_from_stream_bytes()
 */
string *read_file_bytes(arena_t *arena, const string *path, const u64 bytes);

//
//
//

/**
 * @brief Write @c contents to file stream.
 *
 * @details By writing `len` characters from @c contents to that file thats
 * associated with @c stream using <a
 * href="https://en.cppreference.com/w/c/io/fwrite"
 * target="_blank">fwrite()</a>.
 *
 * @pre
 * - @c stream and @c contents must be valid, and cannot be `null`.
 * - @c stream should be preferably open with "wb".
 *
 * @param stream The file stream to write to.
 * @param contents The \ref string to write to the file.
 *
 * @return The number of bytes written.
 *
 * @see <a href="https://en.cppreference.com/w/c/io/fwrite"
 * target="_blank">fwrite()</a>
 */
u64 write_to_file_stream(FILE *stream, const string *contents);

/**
 * @brief Write @c contents to file path.
 *
 * @details By writing `len` characters from @c contents to the file at @c path
 * using \ref write_to_file_stream().
 *
 * @pre @c path and @c contents must be valid, and cannot be `null`.
 *
 * @param path The path to the file to write to.
 * @param contents The \ref string to write to the file.
 *
 * @return The number of bytes written.
 *
 * @see \ref write_to_file_stream()
 */
u64 write_to_file(const string *path, const string *contents);

///
// Write to file by bytes
//

/**
 * @brief Write @c bytes from @c contents to @c stream.
 *
 * @details By writing @c bytes from @c contents to the file stream
 * provided by @c stream using
 * <a href="https://en.cppreference.com/w/c/io/fwrite" target="_blank">
 * fwrite() </a>.
 *
 * @pre
 *  - @c stream, @c contents, and @c bytes must be valid, and cannot be `null`
 * nor `0`.
 *  - @c stream should be preferably open with "wb".
 *
 * @param stream The file stream to write to.
 * @param contents The \ref string to write to the file.
 * @param bytes The number of bytes to write.
 *
 * @return The number of bytes written.
 *
 * @see <a href="https://en.cppreference.com/w/c/io/fwrite"
 * target="_blank">fwrite()</a>
 */
u64 write_to_file_stream_bytes(FILE *stream, const string *contents,
                               const u64 bytes);

/**
 * @brief Write @c bytes from @contents to a file at @c path.
 *
 * @details By writing @c bytes from @c contents to the file at @c path using
 * \ref write_to_file_stream_bytes(), and then returning the result.
 *
 * @pre @c path and @c contents must be valid, and cannot be `null`.
 *
 * @param path The path to the file to write to.
 * @param contents The \ref string to write to the file.
 *
 * @return The number of bytes written.
 *
 * @see \ref write_to_file_stream_bytes()
 */
u64 write_to_file_bytes(const string *path, const string *contents,
                        const u64 bytes);

//
//
//

#endif // !HTILS_FILE_H
