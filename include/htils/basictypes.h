#ifndef HTILS_BASICTYPES_H
#define HTILS_BASICTYPES_H

/***********************************/

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

/***********************************/

/** Alias of uint8_t for better naming conventions. */
typedef uint8_t u8;

/** Alias of uint16_t for better naming conventions. */
typedef uint16_t u16;

/** Alias of uint32_t for better naming conventions. */
typedef uint32_t u32;

/** Alias of uint64_t for better naming conventions. */
typedef uint64_t u64;

/** Alias of int8_t for better naming conventions. */
typedef int8_t i8;

/** Alias of int16_t for better naming conventions. */
typedef int16_t i16;

/** Alias of int32_t for better naming conventions. */
typedef int32_t i32;

/** Alias of int64_t for better naming conventions. */
typedef int64_t i64;

/** Alias of float for better naming conventions. */
typedef float f32;

/** Alias of double for better naming conventions. */
typedef double f64;

/** Boolean, stored as a 32-bit int (cheaper to pass/return than \c _Bool). */
typedef u32 b32;

/** Defines true as 1. */
#ifndef true
#define true (1)
#endif

/** Defines false as 0. */
#ifndef false
#define false (0)
#endif

/** Make NULL lower-case. */
#define null NULL

/** Character; htils' char alias. */
typedef char cstr;

/** Unsigned character; htils' unsigned char alias. */
typedef unsigned char ucstr;

/** Aliases that mark a value as nullable by name when \c USE_NULLABLE_TYPES is
 * defined. */
#ifdef USE_NULLABLE_TYPES
typedef cstr cstr_nullable;
typedef ucstr ucstr_nullable;
#endif

#endif // !HTILS_BASICTYPES_H
