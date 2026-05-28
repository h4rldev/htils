#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//
//

#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/path.h>
#include <htils/string.h>

#if defined(_WIN32)
#include <shellapi.h>
#include <windows.h>
#elif defined(__linux__)
#include <sys/stat.h>
#include <unistd.h>
#else
#error "Unsupported platform."
#endif

//
//
//

static inline b32 is_sep(u8 c) { return c == '/' || c == '\\'; }

//
//
//

string *path_canonical(arena_t *arena, const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(path->len > 0 && "Path cannot be empty.");
  htils_assert(path->base && "Path base cannot be null.");

  const cstr *path_cstr = string_to_cstr(path);
  cstr *res = realpath(path_cstr, null);
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
  htils_assert(path->base && "Path base cannot be null.");

  u8 *end = path->base + path->len;
  while (end > path->base && is_sep(*(end - 1)))
    end--;

  u8 *start = end;
  while (start > path->base && !is_sep((*(start - 1))))
    start--;

  string *out = string_new(arena, (u64)(end - start));
  memcpy(out->base, start, out->len);

  return out;
}

string *path_dirname(arena_t *arena, const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(arena != null && "Arena cannot be null.");

  htils_assert(path->len > 0 && "Path cannot be empty.");
  htils_assert(path->base && "Path base cannot be null.");

  u8 *end = path->base + path->len;
  while (end > path->base && is_sep(*(end - 1)))
    end--;

  u8 *last_sep = end;
  while (last_sep > path->base && !is_sep(*(last_sep - 1)))
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
  htils_assert(path->base && "Path base cannot be null.");

  u8 *end = path->base + path->len;
  while (end > path->base && is_sep(*(end - 1)))
    end--;

  u8 *sep = end;
  while (sep > path->base && !is_sep(*(sep - 1)))
    sep--;

  u8 *dot = null;
  for (u8 *p = end - 1; p >= sep; p--) {
    if (*p == '.') {
      dot = p;
      break;
    }
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
  htils_assert(path->base && "Path base cannot be null.");

  u8 *end = path->base + path->len;

  while (end > path->base && is_sep(*(end - 1)))
    end--;

  u8 *sep = end;
  while (sep > path->base && !is_sep(*(sep - 1)))
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
  htils_assert(first->base && "First path base cannot be null.");
  htils_assert(second->len > 0 && "Second path cannot be empty.");
  htils_assert(second->base && "Second path base cannot be null.");

  u8 first_ends_sep = (first->len > 0) && is_sep(first->base[first->len - 1]);
  u8 second_starts_sep = (second->len > 0) && is_sep(second->base[0]);

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

//
//
//

b32 make_dir(const string *path) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");
  htils_assert(path->base && "Path base cannot be null.");

#if defined(__linux__)
  int res = mkdir(string_to_cstr(path), 0755);
  return res == 0;
#elif defined(_WIN32)
  int res = CreateDirectory(string_to_cstr(path), null);
  return res != 0;
#else
#error "Unsupported platform."
#endif
}

b32 does_path_exist(const string *path) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");
  htils_assert(path->base && "Path base cannot be null.");

#if defined(__linux__)
  struct stat sb;
  return stat(string_to_cstr(path), &sb) == 0;
#elif defined(_WIN32)
  DWORD attrs = GetFileAttributes(string_to_cstr(path));
  return attrs != INVALID_FILE_ATTRIBUTES;
#else
#error "Unsupported platform."
#endif
}

#if defined(_WIN32)

/** Type-alias of wchar_t to wcstr for type similarity reasons */
typedef wchar_t wcstr;

/**
 * @brief Converts a cstr to a wcstr.
 *
 * @details By converting the cstr to a wcstr using `MultiByteToWideChar()` with
 * the `CP_ACP` code page, making sure the size is correct.
 *
 * @param arena The arena to allocate the wcstr from.
 * @param cstr The cstr to convert.
 *
 * @return The converted wcstr.
 */
wcstr *cstr_to_wcstr(arena_t *arena, const cstr *cstr) {
  htils_assert(cstr != null && "C-String cannot be null.");

  UINT code_page = CP_ACP;
  u64 size_needed = MultiByteToWideChar(code_page, 0, cstr, -1, null, 0);
  htils_assert(size_needed > 0 && "Failed to get size for wcstr.");

  wcstr *out = arena_alloc(arena, wcstr, size_needed);
  htils_assert(MultiByteToWideChar(code_page, 0, cstr, -1, out, size_needed) >
                   0 &&
               "Failed to convert cstr to wcstr.");

  return out;
}

#endif

b32 path_remove(const string *path) {
  htils_assert(path != null && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");

#if defined(__linux__)
  return remove(string_to_cstr(path)) == 0;
#elif defined(_WIN32)
  cstr *path_cstr = string_to_cstr(path);
  wcstr *path_wcstr = cstr_to_wcstr(arena, path_cstr);

  wcstr sz_buf[MAX_PATH + 2];
  wcsncpy_s(sz_buf, MAX_PATH + 1, path_wcstr, _TRUNCATE);
  sz_buf[wcslen(sz_buf) + 1] = L'\0';

  SHFILEOPSTRUCTW fop = {0};
  fop.wFunc = FO_DELETE;
  fop.pFrom = sz_buf;
  fop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
  fop.fFlags |= FOF_ALLOWUNDO;

  return (SHFileOperationW(&fop) == 0 && !fop.fAnyOperationsAborted);
#else
#error "Unsupported platform."
#endif
}
