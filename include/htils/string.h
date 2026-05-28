#ifndef HTILS_STRING_H
#define HTILS_STRING_H

#include <htils/arena.h>
#include <htils/basictypes.h>

//
//
//

/** Helper macro for printing strings. */
#define print_string ".*s"
/** Helper macro that works with \ref print_string. */
#define print_string_arg(string) (int)(string)->len, (cstr *)(string)->base

//
//
//

/**
 * @brief a \ref string.
 *
 * @param base The base of the \ref string.
 * @param len The length of the \ref string.
 */
typedef struct _string {
  u8 *base;
  u64 len;
} string;

/**
 * @brief A slice of a \ref string
 *
 * @param base The base of the \ref string
 * @param len The length of the \ref string
 */
typedef string string_slice;

#ifdef USE_NULLABLE_TYPES
typedef string_t string_nullable_t;
#endif

//
//
//

/**
 * @brief Creates a new \ref string.
 *
 * @details Allocates a new \ref string using \ref arena_alloc(), with the given
 * length, it's contents are null by default.
 *
 * @param arena The \ref arena to allocate from.
 * @param len The length for the \ref string.
 *
 * @pre
 * - @c arena must be valid and cannot be `null`.
 * - @c len must be greater than 0.
 *
 * @return A pointer to the new \ref string.
 *
 * @see \ref arena_alloc()
 */
string *string_new(arena_t *arena, const u64 len);

//
//
//

/**
 * @brief Duplicates @c from to a new \ref string.
 *
 * @details Allocates a new \ref string with \ref string_new(), and copies the
 * contents to the new string.
 *
 * @param arena The \ref arena to allocate from.
 * @param from The \ref string to duplicate.
 *
 * @pre @c arena and @c from must be valid and cannot be `null`.
 *
 * @return A pointer to the new \ref string.
 *
 * @see \ref string_new()
 */
string *string_dup(arena_t *arena, const string *from);

//
//
//

/**
 * @brief Create a \ref string from a C-string.
 *
 * @details Since \ref string is a wrapper around the C-string pretty much, It
 * simply creates a new \ref string with \ref string_new(), and then copies the
 * contents of the C-string to the new string calculating the \ref string's
 * length with <a href="https://en.cppreference.com/c/string/byte/strlen"
 * target="_blank">strlen()</a>.
 *
 * @param arena The \ref arena to allocate from.
 * @param base The C-string to create the \ref string from.
 *
 * @pre @c arena and @c base must be valid and cannot be `null`.
 *
 * @return A pointer to the new string.
 *
 * @see \ref string_new(), <a
 * href="https://en.cppreference.com/c/string/byte/strlen"
 * target="_blank">strlen()</a>.
 */
string *string_from_cstr(arena_t *arena, const cstr *base);

//
//
//

/**
 * @brief Create a \ref string_slice from a C-string.
 *
 * @details We're using string_slice as a term to represent a value
 * instead of a pointer, to have less dependents on arena allocations.
 *
 * @param base The C-string to create the \ref string_slice from.
 * @param len The length of the C-string.
 *
 * @pre
 * - @c base must be valid and cannot be `null`.
 * - @c len must be greater than 0.
 *
 * @return A \ref string_slice.
 */
string_slice string_slice_from_cstr(u8 *base, u64 len);

/**
 * @brief Create a \ref string_slice from a \ref string.
 *
 * @details We're using string_slice as a term to represent a value instead of a
 * pointer, to have less dependents on arena allocations, this is basically the
 * same as \ref string_slice_from_cstr() but for passing a string pointer
 * instead for simply convenience reasons.
 *
 * @param str The \ref string to create the \ref string_slice from.
 *
 * @pre @c str must be valid and cannot be `null`.
 *
 * @return A \ref string_slice.
 */
string_slice string_slice_from_string(string *str);

//
//
//

/**
 * @brief Convert a \ref string to a \ref cstr.
 *
 * @details Casts the `base` param of \ref string and then null-terminates the
 * end of the base, and then returns it.
 *
 * @param str The \ref string to convert.
 *
 * @pre @c str must be valid and cannot be `null`.
 *
 * return A pointer to the \ref cstr.
 *
 * @see \ref string, \ref cstr
 */
cstr *string_to_cstr(const string *str);

//
//
//

/**
 * @brief Concatenates two \ref strings.
 *
 * @details By allocating a new \ref string using \ref arena_alloc() with the
 * size of the 2 \ref strings passed, and then copying them over, and then
 * setting dest to be that new \ref string.
 *
 * @param arena The \ref arena to allocate the new string in.
 * @param dest The \ref string to set to the new string.
 * @param src The \ref string to concatenate to the dest string.
 *
 * @pre @c arena, @c dest, and @c src must be valid and cannot be `null`.
 *
 * @return The length of the new \ref string for verification purposes.
 *
 * @see \ref string, \ref arena, \ref arena_alloc()
 */
u64 string_concat(arena_t *arena, string *dest, const string *src);

/**
 * @brief Concatenate @c len bytes of @c src to @c dest.
 *
 * @details Concatenates @c len bytes of @c src to @c dest, by allocating a
 * new string with the size of @c dest + @c len, and then copying @c dest, then
 * @c len bytes of @c src to the new \ref string, then sets dest to the new
 * \ref string.
 *
 * @param arena The \ref arena to allocate the new string in.
 * @param dest The \ref string to set to the new string.
 * @param src The \ref string to concatenate to the dest string.
 * @param len The length of the bytes to concatenate.
 *
 * @pre
 * - @c arena, @c dest, and @c src must be valid and cannot be `null`.
 * - @c len must be greater than 0.
 *
 * @return The length of the new string for verification purposes.
 *
 * @see \ref string, \ref arena, \ref arena_alloc()
 */
u64 string_concatb(arena_t *arena, string *dest, const string *src,
                   const u64 len);

/**
 * @brief Concatenates a formatted \ref cstr to a \ref string.
 *
 * @details Concatenates a formatted \ref cstr to a \ref string, by first
 * handling the formatting of the variadic args, then calculating the length of
 * them, making a new \ref string and then copying @c dest, and @c fmt to the
 * new
 * \ref string, before setting @c dest.
 *
 * @param arena The \ref arena to allocate the new string in.
 * @param dest The \ref string to set to the new string.
 * @param fmt The format \ref cstr to concatenate to the dest string.
 * @param ... The variadic args to format the \ref string with.
 *
 * @pre @c arena, @c dest, and @c fmt must be valid and cannot be `null`.
 *
 * @return The length of the new \ref string for verification purposes.
 *
 * @see \ref string, \ref arena, \ref arena_alloc(), <a
 * href="https://en.cppreference.com/w/c/io/vfprintf"
 * target="_blank">vsnprintf()</a>.
 */
u64 string_concatf(arena_t *arena, string *dest, const cstr *fmt, ...);

//
//
//

/**
 * @brief Compare two \ref string.
 *
 * @details Compare the length and the base of 2 \ref string to check if
 * they're equal using <a
 * href="https://en.cppreference.com/w/c/string/byte/memcmp"
 * target="_blank">memcmp()</a>.
 *
 * @param first The first \ref string to compare.
 * @param second The second \ref string to compare.
 *
 * @pre @c first and @c second must be valid and cannot be `null`.
 *
 * @return True if equal, false if not.
 *
 * @see \ref string, <a
 * href="https://en.cppreference.com/w/c/string/byte/memcmp"
 * target="_blank">memcmp()</a>
 */
b32 stringcmp(const string *first, const string *second);

/**
 * @brief Compares len bytes of each string.
 *
 * @details Compares len bytes of each string to check if they're equal.
 *
 * @param first The first string to compare.
 * @param second The second string to compare.
 * @param len The length of the bytes to compare.
 *
 * @return True if the \ref strings are equal, false if they're not.
 */
b32 stringcmpb(const string *first, const string *second, const u64 len);

//
//
//

/**
 * @brief Splits a \ref string by a delimiter.
 *
 * @details Create a dynamic array of \ref string in @c darray seperated by @c
 * delim.
 *
 * @param src The \ref string to split.
 * @param delim The delimiter to split by.
 * @param darray The dynamic array to store the split strings in.
 * @param arena The \ref arena to allocate the dynamic array from.
 *
 * @pre
 * - @c src, @c darray, and @c arena must be valid and cannot be `null`.
 * - @c delim must be greater than 0.
 *
 * @return The length of the dynamic array.
 */
u64 string_split(string *src, u8 delim, string ***darray, arena_t *arena);

//
//
//

/**
 * @brief Trims whitespace from the start and end of a \ref string.
 *
 * @param str the \ref string to trim.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
void string_trim(string *str);

/**
 * @brief Trims whitespace from the start of a \ref string.
 *
 * @param str the \ref string to trim.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
void string_trim_left(string *str);

/**
 * @brief Trims whitespace from the end of a \ref string.
 *
 * @param str the \ref string to trim.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
void string_trim_right(string *str);

//
//
//

/**
 * @brief Finds the first occurance of a character in a \ref string.
 *
 * @param haystack the \ref string to search in.
 * @param needle The character to search for.
 *
 * @pre @c haystack and @c needle must be valid and cannot be `null`.
 *
 * @return The index of the first occurance of the character, or -1 if
 * not.
 */
i64 string_findc(string *haystack, u8 needle);

/**
 * @brief Finds the first occurance of a \ref string in a \ref string.
 *
 * @param haystack the \ref string to search in.
 * @param needle the \ref string to search for.
 *
 * @pre @c haystack and @c needle must be valid and cannot be `null`.
 *
 * @return The index of the first occurance of the \ref string, or -1 if not
 * found.
 */
i64 string_find_sstr(string *haystack, string *needle);

//
//
//

#define HTILS_STR(base) string_from_cstr(arena, base)

//
//
//

#endif // !HTILS_STRING_H
