#ifndef HTILS_BASICTYPES_H
#define HTILS_BASICTYPES_H

#include <stddef.h>
#include <stdint.h>

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

/** Make bool more platform efficient */
typedef u32 b32;

/** Redefines true to be `b32`. */
#define true ((b32)1)

/** Redefines false to be `b32`. */
#define false ((b32)0)

/** Make NULL lower-case. */
#define null NULL

/** Make char  more opaque. */
typedef char cstr;

/** Make unsigned char more opaque. */
typedef unsigned char ucstr;

/** Make optional types that just makes it more opaque it's nullable. */
#ifdef USE_NULLABLE_TYPES
typedef u8 u8_nullable;
typedef u16 u16_nullable;
typedef u32 u32_nullable;
typedef u64 u64_nullable;

typedef i8 i8_nullable;
typedef i16 i16_nullable;
typedef i32 i32_nullable;
typedef i64 i64_nullable;

typedef cstr cstr_nullable;
typedef ucstr ucstr_nullable;
#endif

#endif // !HTILS_BASICTYPES_H
