#ifndef HTILS_BASICTYPES_H
#define HTILS_BASICTYPES_H

#include <stddef.h>
#include <stdint.h>

/** Better naming schemes for fixed-width types.  */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/** Make bool more platform efficient */
typedef u32 b32;
#define true ((b32)1)
#define false ((b32)0)

/** Make NULL lower-case. */
#define null NULL

/** Make char * more opaque. */
typedef char cstr;
typedef unsigned char ucstr;

/** Make an optional type that just makes it more opaque it's nullable. */
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
