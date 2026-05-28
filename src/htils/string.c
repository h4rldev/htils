#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//
//
//

#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/basictypes.h>
#include <htils/darray.h>
#include <htils/string.h>

//
//
//

static inline b32 is_whitespace(u8 c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

//
//
//

string *string_new(arena_t *arena, const u64 len) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  string *str = arena_alloc(arena, string, 1);

  str->len = len;
  str->base = arena_alloc(arena, u8, len + 1);

  return str;
}

//
//
//

string *string_dup(arena_t *arena, const string *from) {
  htils_assert(from && "String cannot be null.");
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(from->len > 0 && "String cannot be empty.");

  string *out = string_new(arena, from->len);
  memcpy(out->base, from->base, from->len);

  return out;
}

//
//
//

string *string_from_cstr(arena_t *arena, const cstr *base) {
  htils_assert(base && "Base cannot be null.");
  htils_assert(arena && "Arena cannot be null.");

  u64 len = strlen(base);

  string *out = string_new(arena, len);
  memcpy(out->base, base, len);

  return out;
}

//
//
//

string_slice string_slice_from_cstr(u8 *base, u64 len) {
  htils_assert(base && "Base cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  return (string_slice){.base = base, .len = len};
}

string_slice string_slice_from_string(string *str) {
  htils_assert(str != null && "String cannot be null.");

  return string_slice_from_cstr(str->base, str->len);
}

//
//
//

cstr *string_to_cstr(const string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");
  htils_assert(str->base && "String base cannot be null.");

  cstr *ret = (cstr *)str->base;
  ret[str->len] = '\0';
  return ret;
}

//
//
//

u64 string_concat(arena_t *arena, string *dest, const string *src) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(dest && "dest cannot be null.");

  htils_assert(src && "src cannot be null.");
  htils_assert(src->len > 0 && "Source string cannot be empty.");
  htils_assert(src->base && "Source string base cannot be null.");

  u8 *new_base = arena_alloc(arena, u8, dest->len + src->len);

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, src->base, src->len);

  dest->base = new_base;
  dest->len += src->len;

  return dest->len;
}

u64 string_concatb(arena_t *arena, string *dest, const string *src,
                   const u64 len) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(dest && "Dest cannot be null.");
  htils_assert(src && "Src cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  u8 *new_base = arena_alloc(arena, u8, dest->len + len);

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, src->base, len);

  dest->base = new_base;
  dest->len += len;

  return dest->len;
}

u64 string_concatf(arena_t *arena, string *dest, const cstr *fmt, ...) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(dest && "Dest cannot be null.");
  htils_assert(fmt && "Format cannot be null.");

  va_list args;
  u64 fmt_len_w_args = 0;

  va_start(args, fmt);
  fmt_len_w_args = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  u64 len = dest->len + fmt_len_w_args;

  u8 *new_base = arena_alloc(arena, u8, len + 1);
  va_start(args, fmt);
  u64 written =
      vsnprintf((cstr *)(new_base + dest->len), fmt_len_w_args + 1, fmt, args);
  va_end(args);

  htils_assert(written == fmt_len_w_args && "Failed to write to buffer.");

  memcpy(new_base, dest->base, dest->len);

  dest->base = new_base;
  dest->len = len;

  return len;
}

//
//
//

b32 stringcmp(const string *first, const string *second) {
  htils_assert(first != null && "First string cannot be null.");
  htils_assert(second != null && "Second string cannot be null.");
  htils_assert(first->len > 0 && "First string cannot be empty.");
  htils_assert(second->len > 0 && "Second string cannot be empty.");
  htils_assert(first->base && "First string base cannot be null.");
  htils_assert(second->base && "Second string base cannot be null.");

  if (first->len != second->len)
    return false;

  return stringcmpb(first, second, first->len);
}

b32 stringcmpb(const string *first, const string *second, const u64 len) {
  htils_assert(first != null && "First string cannot be null.");
  htils_assert(second != null && "Second string cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  htils_assert(first->len > 0 && "First string cannot be empty.");
  htils_assert(first->base && "First string base cannot be null.");
  htils_assert(second->len > 0 && "Second string cannot be empty.");
  htils_assert(second->base && "Second string base cannot be null.");

  return memcmp(first->base, second->base, len) == 0;
}

//
//
//

u64 string_split(string *src, u8 delim, string ***darray, arena_t *arena) {
  htils_assert(src != null && "Source string cannot be null.");
  htils_assert(darray != null && "Darray cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");
  htils_assert(delim > 0 && "Delimiter must be greater than 0.");

  htils_assert(src->len > 0 && "Source string cannot be empty.");
  htils_assert(src->base && "Source string base cannot be null.");

  u64 count = 0;
  u8 *start = src->base;
  u8 *end = src->base + src->len;

  for (u8 *p = start; p <= end; p++) {
    if (p == end || *p == delim) {
      u64 len = (u64)(p - start);
      if (len > 0) {
        string *slice = string_new(arena, len);
        memcpy(slice->base, start, len);

        da_append(arena, *darray, slice);
        count++;
        start = p + 1;
      }
    }
  }

  return count;
}

//
//
//

void string_trim(string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");

  string_trim_left(str);
  string_trim_right(str);
}

void string_trim_left(string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");
  htils_assert(str->base && "String base cannot be null.");

  u8 *start = str->base;
  u8 *end = str->base + str->len;

  while (start < end && is_whitespace(*start)) {
    start++;
  }

  str->base = start;
  str->len = (u64)(end - start);
}

void string_trim_right(string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");
  htils_assert(str->base && "String base cannot be null.");

  u8 *start = str->base;
  u8 *end = str->base + str->len;

  while (end > start && is_whitespace(*(end - 1))) {
    end--;
  }

  str->len = (u64)(end - start);
}

//
//
//

i64 string_findc(string *haystack, u8 needle) {
  htils_assert(haystack != null && "Haystack cannot be null.");
  htils_assert(haystack->len > 0 && "Haystack cannot be empty.");
  htils_assert(haystack->base && "Haystack base cannot be null.");

  htils_assert(needle > 0 && "Needle cannot be empty.");

  u8 *found = (u8 *)memchr(haystack->base, needle, haystack->len);
  return found ? (i64)(found - haystack->base) : -1;
}

i64 string_find_sstr(string *haystack, string *needle) {
  htils_assert(haystack != null && "Haystack cannot be null.");
  htils_assert(needle != null && "Needle cannot be null.");
  htils_assert(haystack->len > 0 && "Haystack cannot be empty.");
  htils_assert(haystack->base && "Haystack base cannot be null.");
  htils_assert(needle->len > 0 && "Needle cannot be empty.");
  htils_assert(needle->base && "Needle base cannot be null.");

  for (u8 *p = haystack->base;
       p <= haystack->base + haystack->len - needle->len; p++)
    if (memcmp(needle->base, p, needle->len) == 0)
      return (i64)(p - haystack->base);

  return -1;
}

/// :P
