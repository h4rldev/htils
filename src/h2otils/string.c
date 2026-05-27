#include <stdarg.h>
#include <string.h>

//
//
//

#include <h2o/memory.h>
#include <htils/assert.h>
#include <htils/basictypes.h>

//
//
//

#include <h2otils/darray.h>
#include <h2otils/string.h>

//
//
//

h2o_string *h2o_string_new(h2o_mem_pool_t *pool, const u64 len) {
  htils_assert(len > 0 && "Length must be greater than 0.");
  htils_assert(pool != null && "Pool cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  h2o_string *str = h2o_mem_alloc_pool(pool, h2o_string, 1);

  str->base = h2o_mem_alloc_pool(pool, cstr, len + 1);
  str->len = len;

  return str;
}

h2o_string *h2o_string_from_string(h2o_mem_pool_t *pool, const string *str) {
  htils_assert(str && "String cannot be null.");
  htils_assert(str->base && "String base cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");
  htils_assert(pool && "Pool cannot be null.");

  h2o_string *out = h2o_string_new(pool, str->len);

  memcpy(out->base, str->base, str->len);
  out->len = str->len;

  return out;
}

h2o_string *h2o_string_dup(h2o_mem_pool_t *pool, const h2o_string *from) {
  htils_assert(from && "String cannot be null.");
  htils_assert(from->len > 0 && "String cannot be empty.");
  htils_assert(from->base && "Base cannot be null.");
  htils_assert(pool && "Pool cannot be null.");

  h2o_string *out = h2o_string_new(pool, from->len);

  out->len = from->len;
  memcpy(out->base, from->base, from->len);

  return out;
}

//
//
//

cstr *h2o_string_to_cstr(const h2o_string *str) {
  htils_assert(str && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");
  htils_assert(str->base && "Base cannot be null.");

  cstr *ret = (cstr *)str->base;
  ret[str->len] = '\0';
  return ret;
}

//
//
//

u64 h2o_string_concat(h2o_mem_pool_t *pool, h2o_string *dest,
                      const h2o_string *src) {
  htils_assert(src && "Src cannot be null.");
  htils_assert(src->base && "Src base cannot be null.");
  htils_assert(src->len > 0 && "Src cannot be empty.");
  htils_assert(dest && "Dest cannot be null.");
  htils_assert(dest->base && "Dest base cannot be null.");
  htils_assert(dest->len > 0 && "Dest cannot be empty.");
  htils_assert(pool && "Pool cannot be null.");

  cstr *new_base = h2o_mem_alloc_pool(pool, cstr, dest->len + src->len);

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, src->base, src->len);

  dest->base = new_base;
  dest->len += src->len;

  return dest->len;
}

u64 h2o_string_concatb(h2o_mem_pool_t *pool, h2o_string *dest,
                       const h2o_string *src, const u64 len) {
  htils_assert(src && "Src cannot be null.");
  htils_assert(src->base && "Src base cannot be null.");
  htils_assert(src->len > 0 && "Src cannot be empty.");
  htils_assert(dest && "Dest cannot be null.");
  htils_assert(dest->base && "Dest base cannot be null.");
  htils_assert(dest->len > 0 && "Dest cannot be empty.");
  htils_assert(pool && "Pool cannot be null.");
  htils_assert(len > 0 && "Length must be greater than 0.");

  cstr *new_base = h2o_mem_alloc_pool(pool, cstr, dest->len + len);

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, src->base, len);

  dest->base = new_base;
  dest->len += len;

  return dest->len;
}

u64 h2o_string_concatf(h2o_mem_pool_t *pool, h2o_string *dest, const cstr *fmt,
                       ...) {
  htils_assert(pool && "Pool cannot be null.");
  htils_assert(dest && "Dest cannot be null.");
  htils_assert(dest->base && "Dest base cannot be null.");
  htils_assert(dest->len > 0 && "Dest cannot be empty.");
  htils_assert(fmt && "Format cannot be null.");

  va_list args;
  u64 fmt_len_w_args = 0;

  va_start(args, fmt);
  fmt_len_w_args = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  u64 len = dest->len + fmt_len_w_args;
  cstr *new_base = h2o_mem_alloc_pool(pool, cstr, len);

  cstr *buf = h2o_mem_alloc_pool(pool, cstr, fmt_len_w_args + 1);
  va_start(args, fmt);
  vsnprintf((cstr *)buf, fmt_len_w_args + 1, fmt, args);
  va_end(args);

  memcpy(new_base, dest->base, dest->len);
  memcpy(new_base + dest->len, buf, fmt_len_w_args);

  dest->base = new_base;
  dest->len = len;

  return len;
}

b32 h2o_stringcmp(const h2o_string *first, const h2o_string *second) {
  if (first->len != second->len)
    return false;

  return h2o_memis(first->base, first->len, second->base, second->len);
}

b32 h2o_stringcmpb(const h2o_string *first, const h2o_string *second,
                   const u64 len) {
  return h2o_memis(first->base, len, second->base, len);
}

//
//
//

u64 h2o_string_split(h2o_string *src, cstr delim, h2o_string ***h2o_darray,
                     h2o_mem_pool_t *pool) {
  htils_assert(src && "Source string cannot be null.");
  htils_assert(src->base && "Source string base cannot be null.");
  htils_assert(src->len > 0 && "Source string cannot be empty.");
  htils_assert(h2o_darray && "Darray cannot be null.");
  htils_assert(pool && "Pool cannot be null.");
  htils_assert(delim > 0 && "Delimiter must be greater than 0.");

  u64 count = 0;
  cstr *start = src->base;
  cstr *end = src->base + src->len;

  for (cstr *p = start; p <= end; p++) {
    if (p == end || *p == delim) {
      u64 len = (u64)(p - start);
      if (len > 0) {
        h2o_string *slice = h2o_string_new(pool, len);
        memcpy(slice->base, start, len);

        h2o_da_append(pool, *h2o_darray, slice);
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

void h2o_string_trim(h2o_string *str) {
  htils_assert(str && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");

  h2o_string_trim_left(str);
  h2o_string_trim_right(str);
}

void h2o_string_trim_left(h2o_string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");

  cstr *start = str->base;
  cstr *end = str->base + str->len;

  while (start < end && (*start == ' ' || *start == '\t' || *start == '\n' ||
                         *start == '\r')) {
    start++;
  }

  str->base = start;
  str->len = (u64)(end - start);
}

void h2o_string_trim_right(h2o_string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");

  cstr *start = str->base;
  cstr *end = str->base + str->len;

  while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' ||
                         *(end - 1) == '\n' || *(end - 1) == '\r')) {
    end--;
  }

  str->len = (u64)(end - start);
}

//
//
//

i64 h2o_string_findc(h2o_string *haystack, char needle) {
  htils_assert(haystack != null && "Haystack cannot be null.");
  htils_assert(haystack->len > 0 && "Haystack cannot be empty.");
  htils_assert(haystack->base && "Haystack base cannot be null.");
  htils_assert(needle > 0 && "Needle cannot be empty.");

  for (cstr *p = haystack->base; p < haystack->base + haystack->len; p++)
    if (*p == needle)
      return (i64)(p - haystack->base);

  return -1;
}

i64 h2o_string_find_sstr(h2o_string *haystack, h2o_string *needle) {
  htils_assert(haystack != null && "Haystack cannot be null.");
  htils_assert(haystack->len > 0 && "Haystack cannot be empty.");
  htils_assert(haystack->base && "Haystack base cannot be null.");

  htils_assert(needle != null && "Needle cannot be null.");
  htils_assert(needle->len > 0 && "Needle cannot be empty.");
  htils_assert(needle->base && "Needle base cannot be null.");

  for (cstr *p = haystack->base; p < haystack->base + haystack->len; p++)
    if (memcmp(needle->base, &(*p), needle->len) == 0)
      return (i64)(p - haystack->base);

  return -1;
}
