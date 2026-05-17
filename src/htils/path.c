#include <stdlib.h>
#include <string.h>

//
//
//

#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/path.h>
#include <htils/string.h>

//
//
//

string *path_canonical(arena_t *arena, const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(path->len > 0 && "Path cannot be empty.");

  cstr *res = realpath(string_to_cstr(path), null);
  if (!res)
    return null;

  string *out = string_from_cstr(arena, res);
  free(res);

  return out;
}

string *path_basename(arena_t *arena, const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(path->len > 0 && "Path cannot be empty.");

  u8 *end = path->base + path->len;

  while (end > path->base && (*(end - 1) == '/' || *(end - 1) == '\\'))
    end--;

  u8 *start = end;
  while (start > path->base && (*(start - 1) != '/' && *(start - 1) != '\\'))
    start--;

  string *out = string_new(arena, (u64)(end - start));
  memcpy(out->base, start, out->len);

  return out;
}

string *path_dirname(arena_t *arena, const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(path->len > 0 && "Path cannot be empty.");

  u8 *end = path->base + path->len;
  while (end > path->base && (*(end - 1) == '/' || *(end - 1) == '\\'))
    end--;

  u8 *last_sep = end;
  while (last_sep > path->base &&
         (*(last_sep - 1) != '/' && *(last_sep - 1) != '\\'))
    last_sep--;

  if (last_sep > path->base) {
    u64 dir_len = (u64)(last_sep - 1 - path->base);

    if (dir_len == 0) {
      string *out = string_new(arena, 1);
      memcpy(out->base, path->base, 1);
      return out;
    }

    string *out = string_new(arena, dir_len);
    memcpy(out->base, path->base, dir_len);
    return out;
  }

  if (end > path->base)
    return string_from_cstr(arena, ".");

  string *out = string_new(arena, 1);
  memcpy(out->base, path->base, 1);

  return out;
}

string *path_extension(arena_t *arena, const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(path->len > 0 && "Path cannot be empty.");

  u8 *end = path->base + path->len;
  while (end > path->base && (*(end - 1) == '/' || *(end - 1) == '\\'))
    end--;

  u8 *sep = end;
  while (sep > path->base && (*(sep - 1) != '/' && *(sep - 1) != '\\'))
    sep--;

  u8 *dot = null;
  for (u8 *p = end - 1; p >= sep; p--)
    if (*p == '.') {
      dot = p;
      break;
    }

  if (dot && dot > sep && dot < end - 1) {
    u64 ext_len = (u64)(end - dot);
    string *ext = string_new(arena, ext_len);
    memcpy(ext->base, dot, ext_len);
    return ext;
  }

  return null;
}

string *path_stem(arena_t *arena, const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(path->len > 0 && "Path cannot be empty.");

  u8 *end = path->base + path->len;

  while (end > path->base && (*(end - 1) == '/' || *(end - 1) == '\\'))
    end--;

  u8 *sep = end;
  while (sep > path->base && (*(sep - 1) != '/' && *(sep - 1) != '\\'))
    sep--;

  u8 *dot = null;
  for (u8 *p = end - 1; p >= sep; p--)
    if (*p == '.') {
      if (p > sep)
        dot = p;
      break;
    }

  u64 stem_len = 0;
  if (dot)
    stem_len = (u64)(dot - sep);
  else
    stem_len = (u64)(end - sep);

  string *stem = string_new(arena, stem_len);
  memcpy(stem->base, sep, stem_len);

  return stem;
}

//
//
//

string *path_join(arena_t *arena, const string *first, const string *second) {
  htils_assert(first != null && "First path cannot be null.");
  htils_assert(second != null && "Second path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(first->len > 0 && "First path cannot be empty.");
  htils_assert(second->len > 0 && "Second path cannot be empty.");

  u8 first_ends_sep = (first->len > 0) && (first->base[first->len - 1] == '/' ||
                                           first->base[first->len - 1] == '\\');
  u8 second_starts_sep =
      (second->len > 0) && (second->base[0] == '/' || second->base[0] == '\\');

  u64 total_len = first->len + second->len;
  if (first_ends_sep && second_starts_sep)
    total_len--;
  else if (!first_ends_sep && !second_starts_sep && first->len > 0 &&
           second->len > 0)
    total_len++;

  string *out = string_new(arena, total_len);

  u8 *dst = out->base;
  memcpy(dst, first->base, first->len);
  dst += first->len;

  if (first_ends_sep && second_starts_sep)
    memcpy(dst, second->base + 1, second->len - 1);
  else if (!first_ends_sep && !second_starts_sep && first->len > 0 &&
           second->len > 0) {
    *dst++ = '/';
    memcpy(dst, second->base, second->len);
  } else
    memcpy(dst, second->base, second->len);

  return out;
}
