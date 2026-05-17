#ifndef HTILS_STRING_H
#define HTILS_STRING_H

#include <htils/arena.h>
#include <htils/basictypes.h>

//
//
//

#define print_string ".*s"
#define print_string_arg(string) (int)(string)->len, (cstr *)(string)->base

//
//
//

typedef struct string string;
struct string {
  u8 *base;
  u64 len;
};

#ifdef USE_NULLABLE_TYPES
typedef string_t string_nullable_t;
#endif

//
//
//

/**
 * @brief Creates a new string.
 *
 * @details Heap allocs a new string, with the given length, it's
 * null by default.
 *
 * @param arena The arena to allocate from.
 * @param len The length of the string.
 *
 * @return A pointer to the new string.
 */
string *string_new(arena_t *arena, const u64 len);

//
//
//

/**
 * @brief Duplicates a string.
 *
 * @details Heap dup a string, by allocating a new string to the
 * arena, and copying the contents to the new string.
 *
 * @param arena The arena to allocate from.
 * @param from The string to duplicate.
 *
 * @return A pointer to the new string.
 */
string *string_dup(arena_t *arena, const string *from);

//
//
//

/**
 * @brief Create a string from a C-string.
 *
 * @details Creates a new string from a C-string, it's a wrapper
 * around the C-string pretty much, taking note of its lenth.
 *
 * @param arena The arena to allocate from.
 * @param base The C-string to create the string from.
 *
 * @return A pointer to the new string.
 */
string *string_from_cstr(arena_t *arena, const cstr *base);

//
//
//

/**
 * @brief Convert a string to a C-string.
 *
 * @details Converts a string to a C-string, by simply
 * null-terminating the end of the base, and then returning.
 *
 * @param str The string to convert.
 *
 * return A pointer to the C-string.
 */
cstr *string_to_cstr(const string *str);

//
//
//

/**
 * @brief Concatenates two strings.
 *
 * @details Concatenates two strings, by allocating a new string
 * the size of the 2 strings, and then copying them over, and then setting dest
 * to be that new string.
 *
 * @param arena The arena to allocate the new string in.
 * @param dest The string to set to the new string.
 * @param src The string to concatenate to the dest string.
 *
 * @return The length of the new string for verification purposes.
 */
u64 string_concat(arena_t *arena, string *dest, const string *src);

/**
 * @brief Concatenate len bytes of src to dest.
 *
 * @details Concatenates len bytes of src to dest, by allocating a
 * new string with the size of dest + len, and then copying dest, then len bytes
 * of src to the new string, then sets dest to the new string.
 *
 * @param arena The arena to allocate the new string in.
 * @param dest The string to set to the new string.
 * @param src The string to concatenate to the dest string.
 * @param len The length of the bytes to concatenate.
 *
 * @return The length of the new string for verification purposes.
 */
u64 string_concatb(arena_t *arena, string *dest, const string *src,
                   const u64 len);

/**
 * @brief Concatenates a formatted string to a string.
 *
 * @details Concatenates a formatted string to a string, by first
 * handling the formatting of the extra args, then calculating the length of
 * them, making a new string and then copying dest and the format to the new
 * string, before setting dest.
 *
 * @param arena The arena to allocate the new string in.
 * @param dest The string to set to the new string.
 * @param fmt The format string to concatenate to the dest string.
 * @param ... The extra args to format the string with.
 *
 * @return The length of the new string for verification purposes.
 */
u64 string_concatf(arena_t *arena, string *dest, const cstr *fmt, ...);

//
//
//

/**
 * @brief Compares two strings.
 *
 * @details Compare the length and the data of the 2 strings to check if they're
 * equal.
 *
 * @param first The first string to compare.
 * @param second The second string to compare.
 *
 * @return True if the strings are equal, false if they're not.
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
 * @return True if the strings are equal, false if they're not.
 */
b32 stringcmpb(const string *first, const string *second, const u64 len);

//
//
//

/**
 * @brief Splits a string by a delimiter.
 *
 * @details Splits a string by a delimiter, creating a dynamic array of strings
 * in the darray param.
 *
 * @param src The string to split.
 * @param delim The delimiter to split by.
 * @param darray The dynamic array to store the split strings in.
 * @param arena The arena to allocate the dynamic array from.
 *
 * @return The length of the dynamic array.
 */
u64 string_split(string *src, u8 delim, string ***darray, arena_t *arena);

//
//
//

/**
 * @brief Trims whitespace from the start and end of a string.
 *
 * @param str The string to trim.
 */
void string_trim(string *str);

/**
 * @brief Trims whitespace from the start of a string.
 *
 * @param str The string to trim.
 */
void string_trim_left(string *str);

/**
 * @brief Trims whitespace from the end of a string.
 *
 * @param str The string to trim.
 */
void string_trim_right(string *str);

//
//
//

/**
 * @brief Finds the first occurance of a character in a string.
 *
 * @param haystack The string to search in.
 * @param needle The character to search for.
 *
 * @return The index of the first occurance of the character, or -1 if
 * not.
 */
i64 string_findc(string *haystack, u8 needle);

/**
 * @brief Finds the first occurance of a string in a string.
 *
 * @param haystack The string to search in.
 * @param needle The string to search for.
 *
 * @return The index of the first occurance of the string, or -1 if not found.
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
