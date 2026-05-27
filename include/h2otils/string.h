#ifndef H2OTILS_STRING_H
#define H2OTILS_STRING_H

/**
 * A collection of functions to bridge the gap between htils strings and h2o mem
 * pools.
 */

#include <h2o/memory.h>

//
//
//

#include <htils/string.h>

//
//
//

/** Type-alias for h2o_iovec_t. */
typedef h2o_iovec_t h2o_string;

/**
 * @brief Creates a new \ref h2o_string.
 *
 * @details Allocates a new \ref h2o_string with the given
 * length, it's contents are null by default.
 *
 * @param pool The memory pool to allocate from.
 * @param len The length for the \ref string.
 *
 * @pre
 * - @c pool must be valid and cannot be `null`.
 * - @c len must be greater than 0.
 *
 * @return A pointer to the new \ref h2o_string.
 */
h2o_string *h2o_string_new(h2o_mem_pool_t *pool, const u64 len);

/**
 * @brief Converts a \ref string to a \ref h2o_string.
 *
 * @details By simply making a new h2o_string and copying over the string data.
 *
 * @param pool The memory pool to allocate the string from.
 * @param str The string to convert.
 *
 * @pre @c pool, and @c str must be valid and not null.
 *
 * @return A new \ref h2o_string.
 */
h2o_string *h2o_string_from_string(h2o_mem_pool_t *pool, const string *str);

/**
 * @brief Duplicates @c from to a new \ref h2o_string.
 *
 * @details Allocates a new \ref string with \ref h2o_string_new(), and copies
 * the contents to the new string.
 *
 * @param pool The memory pool to allocate the string from.
 * @param from The \ref h2o_string to duplicate.
 *
 * @pre @c pool and @c from must be valid and cannot be `null`.
 *
 * @return A pointer to the new \ref h2o_string.
 *
 * @see \ref h2o_string_new()
 */
h2o_string *h2o_string_dup(h2o_mem_pool_t *pool, const h2o_string *str);

/**
 * @brief Create a \ref h2o_string from a C-string.
 *
 * @details Since \ref h2o_string is a wrapper around the C-string pretty much,
 * It simply creates a new \ref h2o_string with \ref h2o_string_new(), and then
 * copies the contents of the C-string to the new string calculating the \ref
 * string's length with <a
 * href="https://en.cppreference.com/c/string/byte/strlen"
 * target="_blank">strlen()</a>.
 *
 * @param pool The memory pool to allocate the string from.
 * @param base The C-string to create the \ref h2o_string from.
 *
 * @pre @c pool and @c base must be valid and cannot be `null`.
 *
 * @return A pointer to the new h2o_string.
 *
 * @see \ref h2o_string_new(), <a
 * href="https://en.cppreference.com/c/string/byte/strlen"
 * target="_blank">strlen()</a>.
 */
h2o_string *h2o_string_from_cstr(h2o_mem_pool_t *pool, const cstr *str);

//
//
//

/**
 * @brief Convert a \ref h2o_string to a C-string.
 *
 * @details Gets the `base` param of \ref h2o_string and then null-terminates
 * the end of the base, and then returns it.
 *
 * @param str The \ref h2o_string to convert.
 *
 * @pre @c str must be valid and cannot be `null`.
 *
 * return A pointer to the C-string.
 */
cstr *h2o_string_to_cstr(const h2o_string *str);

//
//
//

/**
 * @brief Concatenates the first \ref h2o_string to the second.
 *
 * @details By allocating a new \ref h2o_string, with the size of both @c dest
 * and @c src, then copying them over to @c dest.
 *
 * @param pool The memory pool to allocate the new string in.
 * @param dest The \ref h2o_string to set to the new string.
 * @param src The \ref h2o_string to concatenate to the dest string.
 * @pre @c pool, @c dest, and @c src must be valid and cannot be `null`.
 *
 * @return The length of the new \ref h2o_string for verification purposes.
 */
u64 h2o_string_concat(h2o_mem_pool_t *pool, h2o_string *dest,
                      const h2o_string *src);

/**
 * @brief Concatenates @c len bytes of @c src to @c dest.
 *
 * @details Concatenates @c len bytes of @c src to @c dest, by allocating a new
 * \ref h2o_string, with the size of @c dest + @c len, then copying @c dest,
 * then @c len bytes of @c src to the new \ref h2o_string, then sets dest to the
 * new \ref h2o_string.
 *
 * @param pool The memory pool to allocate the new string in.
 * @param dest The \ref h2o_string to set to the new string.
 * @param src The \ref h2o_string to concatenate to the dest string.
 * @param len The length of the bytes to concatenate.
 *
 * @pre
 *  - @c pool, @c dest, and @c src must be valid and cannot be `null`.
 *  - @c len must be greater than 0.
 *
 *  @return The length of the new \ref h2o_string for verification purposes.
 */
u64 h2o_string_concatb(h2o_mem_pool_t *pool, h2o_string *dest,
                       const h2o_string *src, const u64 len);

/**
 * @brief Concatenates a formatted C-string to a \ref h2o_string.
 *
 * @details Concatenates a formatted C-string to a \ref h2o_string, by first
 * capturing the format with the args, and then copying them over to a new \ref
 * h2o_string, and then setting @dest to it.
 *
 * @param pool The memory pool to allocate the new string in.
 * @param dest The \ref h2o_string to set to the new string.
 * @param fmt The format C-string to concatenate to the dest string.
 * @param ... The variadic args to format the \ref h2o_string with.
 *
 * @pre @c pool, @c dest, and @c fmt must be valid and cannot be `null`.
 *
 * @return The length of the new \ref h2o_string for verification purposes.
 */
u64 h2o_string_concatf(h2o_mem_pool_t *pool, h2o_string *dest, const cstr *fmt,
                       ...);

//
//
//

/**
 * @brief Compare two \ref h2o_string.
 *
 * @details Compare the length and the base of 2 \ref h2o_string to check if
 * they're equal using h3o_memis().
 *
 * @param first The first \ref h2o_string to compare.
 * @param second The second \ref h2o_string to compare.
 *
 * @pre @c first and @c second must be valid and cannot be `null`.
 *
 * @return True if equal, false if not.
 */
b32 h2o_stringcmp(const h2o_string *first, const h2o_string *second);

/**
 * @brief Compare @c len bytes of each \ref h2o_string.
 *
 * @details By using memcmp, compare @c len bytes of each \ref h2o_string to
 * check if they're equal.
 *
 * @param first The first \ref h2o_string to compare.
 * @param second The second \ref h2o_string to compare.
 * @param len The length of the bytes to compare.
 *
 * @pre
 * - @c first and @c second must be valid and cannot be `null`.
 * - @c len must be greater than 0.
 *
 * @return True if the \ref h2o_strings are equal, false if they're not.
 */
b32 h2o_stringcmpb(const h2o_string *first, const h2o_string *second,
                   const u64 len);

//
//
//

/**
 * @brief Splits a \ref h2o_string by a delimiter.
 *
 * @details Creates a dynamic array of \ref h2o_string in @c darray seperated by
 * the delimiter provided in @c delim.
 *
 * @param src The \ref h2o_string to split.
 * @param delim The delimiter to split by.
 * @param darray The dynamic array to store the split strings in.
 * @param pool The memory pool to allocate the dynamic array from.
 *
 * @pre
 * - @c src, @c darray, and @c pool must be valid and cannot be `null`.
 * - @c delim must be greater than 0.
 *
 * @return The length of the dynamic array.
 */
u64 h2o_string_split(h2o_string *src, char delim, h2o_string ***h2o_darray,
                     h2o_mem_pool_t *pool);

//
//
//

/**
 * @brief Trims whitespace from the start and end of a \ref h2o_string.
 *
 * @details Checks for whitespaces and new-lines and trims them away using
 * pointer arithmetic.
 *
 * @param str the \ref h2o_string to trim.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
void h2o_string_trim(h2o_string *str);

/**
 * @brief Trims whitespace from the start of a \ref h2o_string.
 *
 * @details Checks for whitespaces and new-lines and trims them away using
 * pointer arithmetic.
 *
 * @param str the \ref h2o_string to trim.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
void h2o_string_trim_left(h2o_string *str);

/**
 * @brief Trims whitespace from the end of a \ref h2o_string.
 *
 * @details Checks for whitespaces and new-lines and trims them away using
 * pointer arithmetic.
 *
 * @param str the \ref h2o_string to trim.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
void h2o_string_trim_right(h2o_string *str);

//
//
//

/**
 * @brief Finds the first occurance of a character in a \ref h2o_string.
 *
 * @details Uses looping to find the first occurance of a character in a \ref
 * h2o_string.
 *
 * @param haystack the \ref h2o_string to search in.
 * @param needle The character to search for.
 *
 * @pre
 * - @c haystack must be valid and cannot be `null`.
 * - @c needle must be greater than 0.
 *
 * @return The index of the first occurance of the character, or -1 if not.
 */
i64 h2o_string_findc(h2o_string *haystack, char needle);

/**
 * @brief Finds the first occurance of a \ref h2o_string in a \ref h2o_string.
 *
 * @details Uses looping to find the first occurance of a \ref h2o_string in a
 * \ref h2o_string.
 *
 * @param haystack the \ref h2o_string to search in.
 * @param needle the \ref h2o_string to search for.
 *
 * @pre @c haystack and @c needle must be valid and cannot be `null`.
 *
 * @return The index of the first occurance of the \ref h2o_string, or -1 if not
 */
i64 h2o_string_find_sstr(h2o_string *haystack, h2o_string *needle);

#endif // H2OTILS_STRING_H
