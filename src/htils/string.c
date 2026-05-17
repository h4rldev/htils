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

string *string_new(arena_t *arena, const u64 len) {
  htils_assert(len > 0 && "Length must be greater than 0.");
  htils_assert(arena != null && "Arena cannot be null.");
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
  htils_assert(from != null && "String cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");
  htils_assert(from->len > 0 && "String cannot be empty.");

  string *out = string_new(arena, from->len);
  memcpy(out->base, from->base, from->len);

  return out;
}

//
//
//

string *string_from_cstr(arena_t *arena, const cstr *base) {
  htils_assert(base != null && "Base cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  u64 len = strlen(base);

  string *out = string_new(arena, len);
  memcpy(out->base, base, len);

  return out;
}

//
//
//

cstr *string_to_cstr(const string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");

  cstr *ret = (cstr *)str->base;
  ret[str->len] = '\0';
  return ret;
}

//
//
//

u64 string_concat(arena_t *arena, string *dest, const string *src) {
  htils_assert(arena != null && "Arena cannot be null.");
  htils_assert(dest != null && "dest cannot be null.");
  htils_assert(src != null && "src cannot be null.");

  u8 *new_base = arena_alloc(arena, u8, dest->len + src->len);

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, src->base, src->len);

  dest->base = new_base;
  dest->len += src->len;

  return dest->len;
}

u64 string_concatb(arena_t *arena, string *dest, const string *src,
                   const u64 len) {
  htils_assert(arena != null && "Arena cannot be null.");
  htils_assert(dest != null && "Dest cannot be null.");
  htils_assert(src != null && "Src cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  u8 *new_base = arena_alloc(arena, u8, dest->len + len);

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, src->base, len);

  dest->base = new_base;
  dest->len += len;

  return dest->len;
}

u64 string_concatf(arena_t *arena, string *dest, const cstr *fmt, ...) {
  htils_assert(arena != null && "Arena cannot be null.");
  htils_assert(dest != null && "Dest cannot be null.");
  htils_assert(fmt != null && "Format cannot be null.");

  va_list args;
  u64 fmt_len_w_args = 0;

  va_start(args, fmt);
  fmt_len_w_args = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  u64 len = dest->len + fmt_len_w_args;
  u8 *new_base = arena_alloc(arena, u8, len);

  temp_arena_t temp = temp_arena_new(arena);

  u8 *buf = arena_alloc(temp.arena, u8, fmt_len_w_args + 1);
  va_start(args, fmt);
  u64 written = vsnprintf((cstr *)buf, fmt_len_w_args, fmt, args);
  va_end(args);

  htils_assert(written == fmt_len_w_args && "Failed to write to buffer.");

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, buf, fmt_len_w_args);

  temp_arena_free(temp);

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

  if (first->len != second->len)
    return false;

  return stringcmpb(first, second, first->len);
}

b32 stringcmpb(const string *first, const string *second, const u64 len) {
  htils_assert(first != null && "First string cannot be null.");
  htils_assert(second != null && "Second string cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  htils_assert(first->len > 0 && "First string cannot be empty.");
  htils_assert(second->len > 0 && "Second string cannot be empty.");

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

  u64 count = 0;
  u8 *start = src->base;
  u8 *end = src->base + src->len;

  for (u8 *p = start; p <= end; p++) {
    if (p == end || *p == delim) {
      fprintf(stderr, "Appending...\n");

      string *slice = string_new(arena, (u64)(p - start));
      slice->len = (u64)(p - start);
      slice->base = start;

      da_append(arena, *darray, slice);
      count++;
      start = p + 1;
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

  u8 *start = str->base;
  u8 *end = str->base + str->len;

  while (start < end && (*start == ' ' || *start == '\t' || *start == '\n' ||
                         *start == '\r')) {
    start++;
  }

  str->base = start;
  str->len = (u64)(end - start);
}

void string_trim_right(string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");

  u8 *start = str->base;
  u8 *end = str->base + str->len;

  while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' ||
                         *(end - 1) == '\n' || *(end - 1) == '\r')) {
    end--;
  }

  str->len = (u64)(end - start);
}

/// :P
