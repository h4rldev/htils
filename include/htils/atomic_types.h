#ifndef HTILS_ATOMIC_TYPES_H
#define HTILS_ATOMIC_TYPES_H

#include <stdatomic.h>

#include <htils/basictypes.h>

/** Atomic variant of u8. */
typedef _Atomic u8 atomic_u8;

/** Atomic variant of u16. */
typedef _Atomic u16 atomic_u16;

/** Atomic variant of u32. */
typedef _Atomic u32 atomic_u32;

/** Atomic variant of u64. */
typedef _Atomic u64 atomic_u64;

/** Atomic variant of i8. */
typedef _Atomic i8 atomic_i8;

/** Atomic variant of i16. */
typedef _Atomic i16 atomic_i16;

/** Atomic variant of i32. */
typedef _Atomic i32 atomic_i32;

/** Atomic variant of i64. */
typedef _Atomic i64 atomic_i64;

/** Atomic variant of f32. */
typedef _Atomic f32 atomic_f32;

/** Atomic variant of f64. */
typedef _Atomic f64 atomic_f64;

/** Atomic variant of b32. */
typedef atomic_u32 atomic_b32;

#endif // !HTILS_ATOMIC_TYPES_H
