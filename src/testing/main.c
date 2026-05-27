#include <string.h>

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/cli.h>
#include <htils/darray.h>
#include <htils/dotenv.h>
#include <htils/file.h>
#include <htils/path.h>
#include <htils/string.h>
#include <htils/stringmap.h>
#include <htils/test.h>

#define REMOVE false

HTILS_TEST(string_new) {
  string *str = string_new(arena, 4);

  HTILS_TEST_ASSERT(
      str, "String is null, something went wrong on making the string.");
  HTILS_TEST_ASSERT(
      str->len == 4,
      "String length is not 4, something went wrong on making the string.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_dup) {
  string *str = string_new(arena, 4);
  HTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");
  memcpy(str->base, "meow", 4);

  string *dup = string_dup(arena, str);
  HTILS_TEST_ASSERT(dup, "Dup string is null, something went wrong.");
  HTILS_TEST_ASSERT(dup->len == 4, "Dup string length is not 4.");

  HTILS_TEST_ASSERT(memcmp(dup->base, str->base, 4) == 0,
                    "Strings are not the same");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_from_cstr) {
  string *str = string_from_cstr(arena, "meow");
  HTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");

  HTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                    "string is not 'meow', when it was expected.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_to_cstr) {
  string *str = string_from_cstr(arena, "meow");
  HTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");

  HTILS_TEST_ASSERT(memcmp(string_to_cstr(str), "meow", 4) == 0,
                    "string is not 'meow', when it was expected.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_concat) {
  string *str1 = string_from_cstr(arena, "meow");
  string *str2 = string_from_cstr(arena, "meow2");

  HTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str2, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  HTILS_TEST_ASSERT(str2->len == 5, "String length is not 5.");

  u64 written = string_concat(arena, str1, str2);
  HTILS_TEST_ASSERT(written == 9, "Strings were not concatenated properly.");
  HTILS_TEST_ASSERT(memcmp(str1->base, "meowmeow2", 9) == 0,
                    "Strings are not concatenated properly.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_concatb) {
  string *str1 = string_from_cstr(arena, "meow");
  string *str2 = string_from_cstr(arena, "meow2");

  HTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str2, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  HTILS_TEST_ASSERT(str2->len == 5, "String length is not 5.");

  u64 written = string_concatb(arena, str1, str2, 2);
  HTILS_TEST_ASSERT(written == 6, "Strings were not concatenated properly.");

  HTILS_TEST_ASSERT(memcmp(str1->base, "meowme", 6) == 0,
                    "Failed to concat properly.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_concatf) {
  string *str1 = string_from_cstr(arena, "meow");
  HTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");

  u64 written = string_concatf(arena, str1, "%s123", "meow2");
  HTILS_TEST_ASSERT(written == 12, "Strings were not concatenated properly.");
  HTILS_TEST_ASSERT(memcmp(str1->base, "meowmeow2123", 12) == 0,
                    "String wasnt concatenated to properly.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(stringcmp) {
  string *str1 = string_from_cstr(arena, "meow");
  string *str2 = string_dup(arena, str1);

  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str2, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  HTILS_TEST_ASSERT(str2->len == 4, "String length is not 4.");

  b32 cmp = stringcmp(str1, str2);
  HTILS_TEST_ASSERT(cmp == true, "Strings are not equal.");

  string *str3 = string_from_cstr(arena, "meow2");
  HTILS_TEST_ASSERT(str3, "String is null.");
  HTILS_TEST_ASSERT(str3->len == 5, "String length is not 5.");

  cmp = stringcmp(str1, str3);
  HTILS_TEST_ASSERT(cmp == false, "Strings are equal when they shouldn't.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(stringcmpb) {
  string *str1 = string_from_cstr(arena, "meow");
  string *str2 = string_dup(arena, str1);

  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str2, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  HTILS_TEST_ASSERT(str2->len == 4, "String length is not 4.");

  b32 cmp = stringcmpb(str1, str2, 4);
  HTILS_TEST_ASSERT(cmp == true, "Strings are not equal.");

  cmp = stringcmpb(str1, str2, 1);
  HTILS_TEST_ASSERT(cmp == true, "First 1 byte of strings are not equal.");

  cmp = stringcmpb(str1, str2, 2);
  HTILS_TEST_ASSERT(cmp == true, "First 2 bytes of strings are not equal.");

  cmp = stringcmpb(str1, str2, 3);
  HTILS_TEST_ASSERT(cmp == true, "First 3 bytes of strings are not equal.");

  string *str3 = string_from_cstr(arena, "woof");

  HTILS_TEST_ASSERT(str3, "String is null.");
  HTILS_TEST_ASSERT(str3->len == 4, "String length is not 4.");

  b32 cmp2 = stringcmpb(str1, str3, 4);
  HTILS_TEST_ASSERT(cmp2 == false, "Strings are equal when they shouldn't.");

  cmp2 = stringcmpb(str1, str3, 3);
  HTILS_TEST_ASSERT(cmp2 == false,
                    "First 3 bytes of strings are equal when they shouldn't.");

  cmp2 = stringcmpb(str1, str3, 2);
  HTILS_TEST_ASSERT(cmp2 == false,
                    "First 2 bytes of strings are equal when they shouldn't.");

  cmp2 = stringcmpb(str1, str3, 1);
  HTILS_TEST_ASSERT(cmp2 == false,
                    "First 1 byte of strings are equal when they shouldn't.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_trim) {
  string *str1 = string_from_cstr(arena, "  meow  ");
  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 8, "String length is not 8.");

  string_trim(str1);

  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  HTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                    "String wasn't trimmed.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_trim_left) {
  string *str1 = string_from_cstr(arena, "  meow  ");
  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 8, "String length is not 8.");

  string_trim_left(str1);

  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 6, "String length is not 6.");
  HTILS_TEST_ASSERT(memcmp(str1->base, "meow  ", 6) == 0,
                    "String wasn't trimmed.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_trim_right) {
  string *str1 = string_from_cstr(arena, "  meow  ");
  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 8, "String length is not 8.");

  string_trim_right(str1);

  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 6, "String length is not 6.");
  HTILS_TEST_ASSERT(memcmp(str1->base, "  meow", 6) == 0,
                    "String wasn't trimmed.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_findc) {
  string *str1 = string_from_cstr(arena, "meow");
  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");

  i64 index = string_findc(str1, 'e');
  HTILS_TEST_ASSERT(index == 1, "String wasn't found.");

  index = string_findc(str1, 'm');
  HTILS_TEST_ASSERT(index == 0, "String wasn't found.");

  index = string_findc(str1, 'q');
  HTILS_TEST_ASSERT(index == -1, "String was found.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(string_find_sstr) {
  string *str1 = string_from_cstr(arena, "meow");
  HTILS_TEST_ASSERT(str1, "String is null.");
  HTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");

  i64 index = string_find_sstr(str1, string_from_cstr(arena, "me"));
  HTILS_TEST_ASSERT(index == 0, "String wasn't found.");

  index = string_find_sstr(str1, string_from_cstr(arena, "mew"));
  HTILS_TEST_ASSERT(index == -1, "String was found.");

  index = string_find_sstr(str1, string_from_cstr(arena, "ew"));
  HTILS_TEST_ASSERT(index == -1, "String was found.");

  index = string_find_sstr(str1, string_from_cstr(arena, "q"));
  HTILS_TEST_ASSERT(index == -1, "String was found.");

  return HTILS_TEST_PASS;
}

//
//
//

HTILS_TEST(da_new) {
  string **strs = {0};
  da_new(arena, strs, 1);

  HTILS_TEST_ASSERT(da_cap(strs) == 1, "Dynamic array capacity is not 1.");
  HTILS_TEST_ASSERT(da_len(strs) == 0, "Dynamic array length is not 0.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(da_append) {
  string **strs = {0};
  da_new(arena, strs, 1);

  HTILS_TEST_ASSERT(da_len(strs) == 0, "Dynamic array length is not 0.");
  HTILS_TEST_ASSERT(da_cap(strs) == 1, "Dynamic array capacity is not 1.");

  da_append(arena, strs, string_from_cstr(arena, "meow"));
  da_append(arena, strs, string_from_cstr(arena, "meow2"));
  da_append(arena, strs, string_from_cstr(arena, "meow3"));

  HTILS_TEST_ASSERT(da_len(strs) == 3, "Dynamic array length is not 3.");
  HTILS_TEST_ASSERT(da_cap(strs) == 4, "Dynamic array capacity is not 4.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(da_pop) {
  string **strs = {0};
  da_new(arena, strs, 1);

  HTILS_TEST_ASSERT(da_len(strs) == 0, "Dynamic array length is not 0.");
  HTILS_TEST_ASSERT(da_cap(strs) == 1, "Dynamic array capacity is not 1.");

  da_append(arena, strs, string_from_cstr(arena, "meow"));
  da_append(arena, strs, string_from_cstr(arena, "meow2"));
  da_append(arena, strs, string_from_cstr(arena, "meow3"));

  HTILS_TEST_ASSERT(da_len(strs) == 3, "Dynamic array length is not 3.");
  HTILS_TEST_ASSERT(da_cap(strs) == 4, "Dynamic array capacity is not 4.");

  da_pop(strs);

  HTILS_TEST_ASSERT(da_len(strs) == 2, "Dynamic array length is not 2.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(da_last) {
  string **strs = {0};
  da_new(arena, strs, 1);

  HTILS_TEST_ASSERT(da_len(strs) == 0, "Dynamic array length is not 0.");
  HTILS_TEST_ASSERT(da_cap(strs) == 1, "Dynamic array capacity is not 1.");

  da_append(arena, strs, string_from_cstr(arena, "meow"));
  da_append(arena, strs, string_from_cstr(arena, "meow2"));
  da_append(arena, strs, string_from_cstr(arena, "meow3"));

  HTILS_TEST_ASSERT(da_len(strs) == 3, "Dynamic array length is not 3.");
  HTILS_TEST_ASSERT(da_cap(strs) == 4, "Dynamic array capacity is not 4.");

  string *last = da_last(strs);
  HTILS_TEST_ASSERT(last, "Last string is null.");
  HTILS_TEST_ASSERT(last->len == 5, "Last string length is not 3.");
  HTILS_TEST_ASSERT(memcmp(last->base, "meow3", 5) == 0,
                    "Last string is not meow3.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(da_clear) {
  string **strs = {0};
  da_new(arena, strs, 1);

  HTILS_TEST_ASSERT(da_len(strs) == 0, "Dynamic array length is not 0.");
  HTILS_TEST_ASSERT(da_cap(strs) == 1, "Dynamic array capacity is not 1.");

  da_append(arena, strs, string_from_cstr(arena, "meow"));
  da_append(arena, strs, string_from_cstr(arena, "meow2"));
  da_append(arena, strs, string_from_cstr(arena, "meow3"));

  HTILS_TEST_ASSERT(da_len(strs) == 3, "Dynamic array length is not 3.");
  HTILS_TEST_ASSERT(da_cap(strs) == 4, "Dynamic array capacity is not 4.");

  da_clear(strs);
  HTILS_TEST_ASSERT(da_len(strs) == 0, "Dynamic array length is not 0.");

  return HTILS_TEST_PASS;
}

//
//
//

HTILS_TEST(string_split) {
  string **strs = {0};
  da_new(arena, strs, 1);

  string *str1 = string_from_cstr(arena, "meow");
  u64 len = string_split(str1, 'e', &strs, arena);
  HTILS_TEST_ASSERT(len == 2, "Strings were not split properly.");

  HTILS_TEST_ASSERT(strs[0], "String is null.");
  HTILS_TEST_ASSERT(strs[0]->len == 1, "String length is not 3.");
  HTILS_TEST_ASSERT(memcmp(strs[0]->base, "m", 1) == 0,
                    "String didn't split properly.");

  HTILS_TEST_ASSERT(strs[1], "String is null.");
  HTILS_TEST_ASSERT(strs[1]->len == 2, "String length is not 2.");
  HTILS_TEST_ASSERT(memcmp(strs[1]->base, "ow", 2) == 0,
                    "String didn't split properly.");

  return HTILS_TEST_PASS;
}

//
//
//

HTILS_TEST(arena_free) {
  arena_t *new_arena = arena_new(MiB(1), KiB(128));
  HTILS_TEST_ASSERT(new_arena, "Failed to create new arena.");
  arena_free(new_arena);

  return HTILS_TEST_PASS;
}

HTILS_TEST(arena_alloc) {
  string *str = string_new(arena, 4);
  HTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  HTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");

  u8 *new = arena_alloc(arena, u8, 4);
  HTILS_TEST_ASSERT(new, "Failed to allocate memory.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(arena_dealloc) {
  (void)arena_alloc(arena, u8, 4);
  u64 pos = arena->pos;
  arena_dealloc(arena, u8, 4);
  HTILS_TEST_ASSERT(arena->pos == pos - (4 * sizeof(u8)), "Failed to dealloc.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(arena_dealloc_to) {
  arena_t *new_arena = arena_new(MiB(1), KiB(128));
  (void)arena_alloc(new_arena, u8, 4);

  arena_dealloc_to(new_arena, 0);
  HTILS_TEST_ASSERT(new_arena->pos == (sizeof(arena_t)),
                    "Failed to dealloc to 0.");

  arena_free(new_arena);
  return HTILS_TEST_PASS;
}

HTILS_TEST(arena_clear) {
  arena_t *new_arena = arena_new(MiB(1), KiB(128));
  (void)arena_alloc(new_arena, u8, 4);

  arena_clear(new_arena);
  HTILS_TEST_ASSERT(new_arena->pos == (sizeof(arena_t)),
                    "Failed to clear to base.");

  arena_free(new_arena);
  return HTILS_TEST_PASS;
}

//
//
//

HTILS_TEST(temp_arena_new) {
  temp_arena_t temp = temp_arena_new(arena);
  HTILS_TEST_ASSERT(temp.arena == arena, "Arena is not the same.");
  HTILS_TEST_ASSERT(temp.start_pos == arena->pos, "Start pos is not the same.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(temp_arena_free) {
  temp_arena_t temp = temp_arena_new(arena);
  u64 pos = arena->pos;

  HTILS_TEST_ASSERT(temp.arena == arena, "Arena is not the same.");
  HTILS_TEST_ASSERT(temp.start_pos == arena->pos, "Start pos is not the same.");

  (void)arena_alloc(arena, u8, 4);

  temp_arena_free(temp);
  HTILS_TEST_ASSERT(arena->pos == pos, "Failed to free temp arena.");

  return HTILS_TEST_PASS;
}

//
//
//

HTILS_TEST(stringmap_new) {
  stringmap_t *map = sm_new(arena, 4);

  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 4.");
  HTILS_TEST_ASSERT(map->count == 0, "Stringmap size is not 0.");

  return HTILS_TEST_PASS;
}
HTILS_TEST(stringmap_insert) {
  stringmap_t *map = sm_new(arena, 4);
  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 4.");
  HTILS_TEST_ASSERT(map->count == 0, "Stringmap size is not 0.");

  string *key = string_from_cstr(arena, "meow");
  string *val = string_from_cstr(arena, "meow");
  sm_insert(map, key, val);

  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 8.");
  HTILS_TEST_ASSERT(map->count == 1, "Stringmap size is not 1.");

  return HTILS_TEST_PASS;
}
HTILS_TEST(stringmap_kill) {
  stringmap_t *map = sm_new(arena, 4);
  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 4.");
  HTILS_TEST_ASSERT(map->count == 0, "Stringmap size is not 0.");

  string *key = string_from_cstr(arena, "meow");
  string *val = string_from_cstr(arena, "meow");

  sm_insert(map, key, val);
  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 8.");
  HTILS_TEST_ASSERT(map->count == 1, "Stringmap size is not 1.");

  sm_kill(map, key);
  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 4.");
  HTILS_TEST_ASSERT(map->count == 0, "Stringmap size is not 0.");
  HTILS_TEST_ASSERT(map->dead_entries == 1, "Stringmap dead is not 1.");

  return HTILS_TEST_PASS;
}
HTILS_TEST(stringmap_get) {
  stringmap_t *map = sm_new(arena, 4);
  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 4.");
  HTILS_TEST_ASSERT(map->count == 0, "Stringmap size is not 0.");

  string *key = string_from_cstr(arena, "meow");
  string *val = string_from_cstr(arena, "meow");

  sm_insert(map, key, val);
  HTILS_TEST_ASSERT(map, "Stringmap is null.");
  HTILS_TEST_ASSERT(map->capacity == 4, "Stringmap capacity is not 8.");
  HTILS_TEST_ASSERT(map->count == 1, "Stringmap size is not 1.");

  string *val2 = sm_get(map, key);
  HTILS_TEST_ASSERT(val2, "Stringmap value is null.");
  HTILS_TEST_ASSERT(val2->len == 4, "Stringmap value length is not 4.");
  HTILS_TEST_ASSERT(memcmp(val2->base, "meow", 4) == 0,
                    "Stringmap value is not 'meow'.");

  return HTILS_TEST_PASS;
}

//
//
//

HTILS_TEST(path_canonical) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");
  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  string *canonical = path_canonical(arena, path);
  HTILS_TEST_ASSERT(canonical, "Canonical path is null.");
  HTILS_TEST_ASSERT(canonical->len > 0, "Canonical path length is 0.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(path_basename) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");
  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  string *basename = path_basename(arena, path);
  HTILS_TEST_ASSERT(basename, "Basename is null.");
  HTILS_TEST_ASSERT(basename->len > 0, "Basename length is 0.");
  HTILS_TEST_ASSERT(memcmp(basename->base, "main.c", 6) == 0,
                    "Failed to extract basename.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(path_dirname) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");
  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  string *dirname = path_dirname(arena, path);
  HTILS_TEST_ASSERT(dirname, "Dirname is null.");
  HTILS_TEST_ASSERT(dirname->len > 0, "Dirname length is 0.");
  HTILS_TEST_ASSERT(memcmp(dirname->base, "./src/testing", 14) == 0,
                    "Failed to extract dirname.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(path_extension) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");
  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  string *extension = path_extension(arena, path);
  HTILS_TEST_ASSERT(extension, "Extension is null.");
  HTILS_TEST_ASSERT(extension->len > 0, "Extension length is 0.");
  HTILS_TEST_ASSERT(memcmp(extension->base, ".c", 2) == 0,
                    "Failed to extract extension.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(path_stem) {
  string *path = string_from_cstr(arena, "./src/testing/main.c.file");
  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  string *stem = path_stem(arena, path);
  HTILS_TEST_ASSERT(stem, "Stem is null.");
  HTILS_TEST_ASSERT(stem->len > 0, "Stem length is 0.");
  HTILS_TEST_ASSERT(memcmp(stem->base, "main.c", 6) == 0,
                    "Failed to extract stem.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(path_join) {
  string *first = string_from_cstr(arena, "./src/");
  string *second = string_from_cstr(arena, "testing/main.c");

  HTILS_TEST_ASSERT(first, "First path is null.");
  HTILS_TEST_ASSERT(second, "Second path is null.");
  HTILS_TEST_ASSERT(first->len > 0, "First path length is 0.");
  HTILS_TEST_ASSERT(second->len > 0, "Second path length is 0.");

  string *joined = path_join(arena, first, second);
  HTILS_TEST_ASSERT(joined, "Joined path is null.");
  HTILS_TEST_ASSERT(joined->len > 0, "Joined path length is 0.");
  HTILS_TEST_ASSERT(memcmp(joined->base, "./src/testing/main.c", 19) == 0,
                    "Failed to join paths.");

  string *first1 = string_from_cstr(arena, "./src");
  string *second1 = string_from_cstr(arena, "testing/main.c");

  HTILS_TEST_ASSERT(first1, "First path is null.");
  HTILS_TEST_ASSERT(second1, "Second path is null.");
  HTILS_TEST_ASSERT(first1->len > 0, "First path length is 0.");
  HTILS_TEST_ASSERT(second1->len > 0, "Second path length is 0.");

  string *joined1 = path_join(arena, first1, second1);
  HTILS_TEST_ASSERT(joined1, "Joined path is null.");
  HTILS_TEST_ASSERT(joined1->len > 0, "Joined path length is 0.");
  HTILS_TEST_ASSERT(memcmp(joined1->base, "./src/testing/main.c", 19) == 0,
                    "Failed to join paths.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(make_dir) {
  string *path = string_from_cstr(arena, "tests_out");

  if (does_path_exist(path))
    return HTILS_TEST_SKIP;

  b32 made = make_dir(path);
  HTILS_TEST_ASSERT(made, "Failed to make directory.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(file_exists) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  b32 exists = does_path_exist(path);
  HTILS_TEST_ASSERT(exists, "Failed to check if path exists.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(dir_exists) {
  string *path = string_from_cstr(arena, "./src/testing");
  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  b32 exists = does_path_exist(path);
  HTILS_TEST_ASSERT(exists, "Failed to check if path exists.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(remove_dir) {
  string *path = string_from_cstr(arena, "test_make_dir");
  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  b32 made = make_dir(path);
  HTILS_TEST_ASSERT(made, "Failed to make directory.");

  b32 removed = path_remove(path);
  HTILS_TEST_ASSERT(removed, "Failed to remove directory.");

  return HTILS_TEST_PASS;
}

//
//
//

HTILS_TEST(file_size_stream) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  HTILS_TEST_ASSERT(does_path_exist(path), "Path does not exist.");

  FILE *stream = fopen(string_to_cstr(path), "rb");
  HTILS_TEST_ASSERT(stream, "Failed to open file.");

  HTILS_TEST_ASSERT(file_size_stream(stream) > 0, "File size is 0.");
  fclose(stream);

  return HTILS_TEST_PASS;
}

HTILS_TEST(file_size) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  HTILS_TEST_ASSERT(does_path_exist(path), "Path does not exist.");
  HTILS_TEST_ASSERT(file_size(path) > 0, "File size is 0.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(read_file_from_stream) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  HTILS_TEST_ASSERT(does_path_exist(path), "Path does not exist.");

  u64 size = file_size(path);
  HTILS_TEST_ASSERT(size > 0, "File size is 0.");

  FILE *stream = fopen(string_to_cstr(path), "rb");
  HTILS_TEST_ASSERT(stream, "Failed to open file.");

  string *contents = read_file_from_stream(arena, stream);
  HTILS_TEST_ASSERT(contents, "Failed to read file.");
  HTILS_TEST_ASSERT(contents->len == size, "File size is not the same.");
  fclose(stream);

  return HTILS_TEST_PASS;
}

HTILS_TEST(read_file) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  HTILS_TEST_ASSERT(does_path_exist(path), "Path does not exist.");

  u64 size = file_size(path);
  HTILS_TEST_ASSERT(size > 0, "File size is 0.");

  string *contents = read_file(arena, path);
  HTILS_TEST_ASSERT(contents, "Failed to read file.");
  HTILS_TEST_ASSERT(contents->len == size, "File size is not the same.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(read_file_from_stream_bytes) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  HTILS_TEST_ASSERT(does_path_exist(path), "Path does not exist.");

  u64 size = file_size(path);
  HTILS_TEST_ASSERT(size > 0, "File size is 0.");

  u64 bytes_to_read = size / 2;
  FILE *stream = fopen(string_to_cstr(path), "rb");
  HTILS_TEST_ASSERT(stream, "Failed to open file.");

  string *contents = read_file_from_stream_bytes(arena, stream, bytes_to_read);
  HTILS_TEST_ASSERT(contents, "Failed to read file.");
  HTILS_TEST_ASSERT(contents->len == bytes_to_read,
                    "Read size is not the same.");
  fclose(stream);

  return HTILS_TEST_PASS;
}

HTILS_TEST(read_file_bytes) {
  string *path = string_from_cstr(arena, "./src/testing/main.c");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");

  HTILS_TEST_ASSERT(does_path_exist(path), "Path does not exist.");

  u64 size = file_size(path);
  HTILS_TEST_ASSERT(size > 0, "File size is 0.");

  u64 bytes_to_read = size / 2;
  string *contents = read_file_bytes(arena, path, bytes_to_read);
  HTILS_TEST_ASSERT(contents, "Failed to read file.");
  HTILS_TEST_ASSERT(contents->len == bytes_to_read,
                    "Read size is not the same.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(write_to_file_stream) {
  string *path = string_from_cstr(arena, "tests_out/test.txt");
  string *contents = string_from_cstr(arena, "meow\nmeow\nmeow\n");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");
  HTILS_TEST_ASSERT(contents, "Contents is null.");
  HTILS_TEST_ASSERT(contents->len > 0, "Contents length is 0.");

  FILE *stream = fopen(string_to_cstr(path), "wb");
  HTILS_TEST_ASSERT(stream, "Failed to open file.");

  u64 written = write_to_file_stream(stream, contents);
  HTILS_TEST_ASSERT(written == contents->len, "Failed to write to file.");
  fclose(stream);

  return HTILS_TEST_PASS;
}

HTILS_TEST(write_to_file) {
  string *path = string_from_cstr(arena, "tests_out/test.txt");
  string *contents = string_from_cstr(arena, "meow\nmeow\nmeow\n");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");
  HTILS_TEST_ASSERT(contents, "Contents is null.");
  HTILS_TEST_ASSERT(contents->len > 0, "Contents length is 0.");

  u64 written = write_to_file(path, contents);
  HTILS_TEST_ASSERT(written == contents->len, "Failed to write to file.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(write_to_file_stream_bytes) {
  string *path = string_from_cstr(arena, "tests_out/test2.txt");
  string *contents = string_from_cstr(arena, "meow\nmeow\nmeow\n");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");
  HTILS_TEST_ASSERT(contents, "Contents is null.");
  HTILS_TEST_ASSERT(contents->len > 0, "Contents length is 0.");

  FILE *stream = fopen(string_to_cstr(path), "wb");
  HTILS_TEST_ASSERT(stream, "Failed to open file.");

  u64 written =
      write_to_file_stream_bytes(stream, contents, contents->len - 10);
  HTILS_TEST_ASSERT(written == contents->len - 10, "Failed to write to file.");
  fclose(stream);

  return HTILS_TEST_PASS;
}

HTILS_TEST(write_to_file_bytes) {
  string *path = string_from_cstr(arena, "tests_out/test2.txt");
  string *contents = string_from_cstr(arena, "meow\nmeow\nmeow\n");

  HTILS_TEST_ASSERT(path, "Path is null.");
  HTILS_TEST_ASSERT(path->len > 0, "Path length is 0.");
  HTILS_TEST_ASSERT(contents, "Contents is null.");
  HTILS_TEST_ASSERT(contents->len > 0, "Contents length is 0.");

  u64 written = write_to_file_bytes(path, contents, contents->len - 10);
  HTILS_TEST_ASSERT(written == contents->len - 10, "Failed to write to file.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(dotenv_load) {
  string *contents = string_from_cstr(arena, "EGG=\"test\"\nEGG2=\"test2\"");
  u64 written = write_to_file(HTILS_STR("./src/testing/gum.env"), contents);
  HTILS_TEST_ASSERT(written == contents->len, "Failed to write to file.");

  i32 amount = htils_dotenv_load(arena, HTILS_STR("./src/testing/"));
  HTILS_TEST_ASSERT(amount == 2, "Failed to load dotenv.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("test"), HTILS_STR(getenv("EGG"))),
                    "EGG is not 'test'");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("test2"), HTILS_STR(getenv("EGG2"))),
                    "EGG2 is not 'test2'");
  b32 removed = path_remove(HTILS_STR("./src/testing/gum.env"));
  HTILS_TEST_ASSERT(removed, "Failed to remove file.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(cli_new) {
  int argc = 2;
  cstr *argv[2] = {"test", "test2"};

  htils_cli_t *cli =
      cli_new(arena, argc, argv, HTILS_STR("Test cli"),
              HTILS_STR("This is a test cli program for testing cli.h"));

  HTILS_TEST_ASSERT(cli, "Cli is null.");
  HTILS_TEST_ASSERT(cli->argc == argc, "Cli argc is not 2.");
  HTILS_TEST_ASSERT(memcmp(cli->argv, argv, argc * sizeof(cstr *)) == 0,
                    "Cli argv is not correct.");
  HTILS_TEST_ASSERT(cli->cli_name, "Cli name is null.");
  HTILS_TEST_ASSERT(cli->cli_name->len > 0, "Cli name is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("Test cli"), cli->cli_name),
                    "Cli name doesn't match.");
  HTILS_TEST_ASSERT(cli->cli_desc, "Cli desc is null.");
  HTILS_TEST_ASSERT(cli->cli_desc->len > 0, "Cli desc is empty.");
  HTILS_TEST_ASSERT(
      stringcmp(HTILS_STR("This is a test cli program for testing cli.h"),
                cli->cli_desc),
      "Cli desc doesn't match.");
  HTILS_TEST_ASSERT(da_len(cli->options) == 0, "Cli options is not 0.");
  HTILS_TEST_ASSERT(cli->arena == arena, "Cli arena is not the same.");

  return HTILS_TEST_PASS;
}

HTILS_TEST(cli_add) {
  int argc = 2;
  cstr *argv[2] = {"-t", "-g"};

  htils_cli_t *cli =
      cli_new(arena, argc, argv, HTILS_STR("Test cli"),
              HTILS_STR("This is a test cli program for testing cli.h"));

  HTILS_TEST_ASSERT(cli, "Cli is null.");
  HTILS_TEST_ASSERT(cli->argc == argc, "Cli argc is not 2.");
  HTILS_TEST_ASSERT(memcmp(cli->argv, argv, argc * sizeof(cstr *)) == 0,
                    "Cli argv is not correct.");
  HTILS_TEST_ASSERT(cli->cli_name, "Cli name is null.");
  HTILS_TEST_ASSERT(cli->cli_name->len > 0, "Cli name is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("Test cli"), cli->cli_name),
                    "Cli name doesn't match.");
  HTILS_TEST_ASSERT(cli->cli_desc, "Cli desc is null.");
  HTILS_TEST_ASSERT(cli->cli_desc->len > 0, "Cli desc is empty.");
  HTILS_TEST_ASSERT(
      stringcmp(HTILS_STR("This is a test cli program for testing cli.h"),
                cli->cli_desc),
      "Cli desc doesn't match.");
  HTILS_TEST_ASSERT(da_len(cli->options) == 0, "Cli options is not 0.");
  HTILS_TEST_ASSERT(cli->arena == arena, "Cli arena is not the same.");

  cli_add(cli, HTILS_STR("test"), HTILS_STR("Test option"), false);
  cli_add(cli, HTILS_STR("other_test"), HTILS_STR("Test option 2"), true);

  HTILS_TEST_ASSERT(da_len(cli->options) == 2, "Cli options is not 2.");
  HTILS_TEST_ASSERT(cli->options[0]->name, "Option name is null.");
  HTILS_TEST_ASSERT(cli->options[0]->name->len > 0, "Option name is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("test"), cli->options[0]->name),
                    "Option name doesn't match.");
  HTILS_TEST_ASSERT(cli->options[0]->desc, "Option desc is null.");
  HTILS_TEST_ASSERT(cli->options[0]->desc->len > 0, "Option desc is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("Test option"), cli->options[0]->desc),
                    "Option desc doesn't match.");
  HTILS_TEST_ASSERT(cli->options[0]->cli_short, "Option cli_short is null.");
  HTILS_TEST_ASSERT(cli->options[0]->cli_short->len > 0,
                    "Option cli_short is empty.");
  HTILS_TEST_ASSERT(cli->options[0]->cli_short_short == 't',
                    "Option cli_short_short doesn't match.");
  HTILS_TEST_ASSERT(cli->options[0]->has_arg == false,
                    "Option has_arg doesn't match.");

  HTILS_TEST_ASSERT(cli->options[1]->name, "Option name is null.");
  HTILS_TEST_ASSERT(cli->options[1]->name->len > 0, "Option name is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("other_test"), cli->options[1]->name),
                    "Option name doesn't match.");
  HTILS_TEST_ASSERT(cli->options[1]->desc, "Option desc is null.");
  HTILS_TEST_ASSERT(cli->options[1]->desc->len > 0, "Option desc is empty.");
  HTILS_TEST_ASSERT(
      stringcmp(HTILS_STR("Test option 2"), cli->options[1]->desc),
      "Option desc doesn't match.");
  HTILS_TEST_ASSERT(cli->options[1]->cli_short,
                    "Option short cli arg is null.");
  HTILS_TEST_ASSERT(cli->options[1]->cli_short->len > 0,
                    "Option short cli arg is empty.");
  HTILS_TEST_ASSERT(cli->options[1]->cli_short_short == 'o',
                    "Option short cli arg doesn't match.");
  HTILS_TEST_ASSERT(cli->options[1]->has_arg == true,
                    "Option has_arg doesn't match.");

  return HTILS_TEST_PASS;
}
HTILS_TEST(parse_cli) {
  int argc = 2;
  cstr *argv[2] = {"-t", "-g"};

  htils_cli_t *cli =
      cli_new(arena, argc, argv, HTILS_STR("Test cli"),
              HTILS_STR("This is a test cli program for testing cli.h"));

  HTILS_TEST_ASSERT(cli, "Cli is null.");
  HTILS_TEST_ASSERT(cli->argc == argc, "Cli argc is not 2.");
  HTILS_TEST_ASSERT(memcmp(cli->argv, argv, argc * sizeof(cstr *)) == 0,
                    "Cli argv is not correct.");
  HTILS_TEST_ASSERT(cli->cli_name, "Cli name is null.");
  HTILS_TEST_ASSERT(cli->cli_name->len > 0, "Cli name is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("Test cli"), cli->cli_name),
                    "Cli name doesn't match.");
  HTILS_TEST_ASSERT(cli->cli_desc, "Cli desc is null.");
  HTILS_TEST_ASSERT(cli->cli_desc->len > 0, "Cli desc is empty.");
  HTILS_TEST_ASSERT(
      stringcmp(HTILS_STR("This is a test cli program for testing cli.h"),
                cli->cli_desc),
      "Cli desc doesn't match.");
  HTILS_TEST_ASSERT(da_len(cli->options) == 0, "Cli options is not 0.");
  HTILS_TEST_ASSERT(cli->arena == arena, "Cli arena is not the same.");

  cli_add(cli, HTILS_STR("test"), HTILS_STR("Test option"), false);
  cli_add(cli, HTILS_STR("other_test"), HTILS_STR("Test option 2"), true);

  HTILS_TEST_ASSERT(da_len(cli->options) == 2, "Cli options is not 2.");
  HTILS_TEST_ASSERT(cli->options[0]->name, "Option name is null.");
  HTILS_TEST_ASSERT(cli->options[0]->name->len > 0, "Option name is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("test"), cli->options[0]->name),
                    "Option name doesn't match.");
  HTILS_TEST_ASSERT(cli->options[0]->desc, "Option desc is null.");
  HTILS_TEST_ASSERT(cli->options[0]->desc->len > 0, "Option desc is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("Test option"), cli->options[0]->desc),
                    "Option desc doesn't match.");
  HTILS_TEST_ASSERT(cli->options[0]->cli_short, "Option cli_short is null.");
  HTILS_TEST_ASSERT(cli->options[0]->cli_short->len > 0,
                    "Option cli_short is empty.");
  HTILS_TEST_ASSERT(cli->options[0]->cli_short_short == 't',
                    "Option cli_short_short doesn't match.");
  HTILS_TEST_ASSERT(cli->options[0]->has_arg == false,
                    "Option has_arg doesn't match.");

  HTILS_TEST_ASSERT(cli->options[1]->name, "Option name is null.");
  HTILS_TEST_ASSERT(cli->options[1]->name->len > 0, "Option name is empty.");
  HTILS_TEST_ASSERT(stringcmp(HTILS_STR("other_test"), cli->options[1]->name),
                    "Option name doesn't match.");
  HTILS_TEST_ASSERT(cli->options[1]->desc, "Option desc is null.");
  HTILS_TEST_ASSERT(cli->options[1]->desc->len > 0, "Option desc is empty.");
  HTILS_TEST_ASSERT(
      stringcmp(HTILS_STR("Test option 2"), cli->options[1]->desc),
      "Option desc doesn't match.");
  HTILS_TEST_ASSERT(cli->options[1]->cli_short,
                    "Option short cli arg is null.");
  HTILS_TEST_ASSERT(cli->options[1]->cli_short->len > 0,
                    "Option short cli arg is empty.");
  HTILS_TEST_ASSERT(cli->options[1]->cli_short_short == 'o',
                    "Option short cli arg doesn't match.");
  HTILS_TEST_ASSERT(cli->options[1]->has_arg == true,
                    "Option has_arg doesn't match.");

  i16 opt;
  while ((opt = parse_cli(cli)) != -1) {
    switch (opt) {
    case 't':
      break;
    case 'o':
      break;
    case '?':
      break;
    default:
      HTILS_TEST_ASSERT(false, "Unexpected option.");
    }
  }

  return HTILS_TEST_PASS;
}

int main(int argc, cstr **argv) {
  arena_t *arena = arena_new(MiB(1), KiB(128));
  u32 failures = 0;
  u32 test_count = 0;

  HTILS_TEST_RUN(string_new);
  HTILS_TEST_RUN(string_dup);
  HTILS_TEST_RUN(string_from_cstr);
  HTILS_TEST_RUN(string_to_cstr);
  HTILS_TEST_RUN(string_concat);
  HTILS_TEST_RUN(string_concatb);
  HTILS_TEST_RUN(string_concatf);
  HTILS_TEST_RUN(stringcmp);
  HTILS_TEST_RUN(stringcmpb);
  HTILS_TEST_RUN(string_trim);
  HTILS_TEST_RUN(string_trim_left);
  HTILS_TEST_RUN(string_trim_right);
  HTILS_TEST_RUN(string_findc);
  HTILS_TEST_RUN(string_find_sstr);

  HTILS_TEST_RUN(da_new);
  HTILS_TEST_RUN(da_append);
  HTILS_TEST_RUN(da_pop);
  HTILS_TEST_RUN(da_last);
  HTILS_TEST_RUN(da_clear);

  HTILS_TEST_RUN(string_split);

  HTILS_TEST_RUN(arena_free);
  HTILS_TEST_RUN(arena_alloc);
  HTILS_TEST_RUN(arena_dealloc);
  HTILS_TEST_RUN(arena_dealloc_to);
  HTILS_TEST_RUN(arena_clear);

  HTILS_TEST_RUN(temp_arena_new);
  HTILS_TEST_RUN(temp_arena_free);

  HTILS_TEST_RUN(stringmap_new);
  HTILS_TEST_RUN(stringmap_insert);
  HTILS_TEST_RUN(stringmap_kill);
  HTILS_TEST_RUN(stringmap_get);

  HTILS_TEST_RUN(path_canonical);
  HTILS_TEST_RUN(path_basename);
  HTILS_TEST_RUN(path_dirname);
  HTILS_TEST_RUN(path_extension);
  HTILS_TEST_RUN(path_stem);
  HTILS_TEST_RUN(path_join);
  HTILS_TEST_RUN(make_dir);
  HTILS_TEST_RUN(remove_dir);
  HTILS_TEST_RUN(file_exists);
  HTILS_TEST_RUN(dir_exists);

  HTILS_TEST_RUN(file_size_stream);
  HTILS_TEST_RUN(file_size);
  HTILS_TEST_RUN(read_file_from_stream);
  HTILS_TEST_RUN(read_file);
  HTILS_TEST_RUN(read_file_from_stream_bytes);
  HTILS_TEST_RUN(read_file_bytes);
  HTILS_TEST_RUN(write_to_file_stream);
  HTILS_TEST_RUN(write_to_file);
  HTILS_TEST_RUN(write_to_file_bytes);
  HTILS_TEST_RUN(write_to_file_stream_bytes);

  HTILS_TEST_RUN(dotenv_load);

  HTILS_TEST_RUN(cli_new);
  HTILS_TEST_RUN(cli_add);
  HTILS_TEST_RUN(parse_cli);

  htils_cli_t *cli =
      cli_new(arena, argc, argv, HTILS_STR("Test cli"),
              HTILS_STR("This is a test cli program for testing cli.h"));

  cli_add(cli, HTILS_STR("test"), HTILS_STR("Test option"), false);
  cli_add(cli, HTILS_STR("other_test"), HTILS_STR("Test option 2"), true);

  i16 opt;
  while ((opt = parse_cli(cli)) != -1) {
    switch (opt) {
    case 't':
      printf("test\n");
      break;
    case 'o':
      printf("other_test: %s\n", cli->optarg);
      break;
    case '?':
      printf("Unknown option: %c\n", cli->optopt);
      cli->usage(cli, argv);
      break;
    case ':':
      switch (cli->optopt) {
      case 'o':
        printf("Option \"o\" requires an argument\n");
        break;
      default:
        printf("Unknown option: %c\n", cli->optopt);
        break;
      }
      break;
    case ';':
      switch (cli->optopt) {
      case 't':
        printf("Option t doesn't require an argument\n");
        break;
      default:
        printf("Unknown option: %c\n", cli->optopt);
        break;
      }
      break;
    default:
      printf("Unexpected option: %c\n", opt);
      break;
    }
  }

  for (i64 i = cli->optidx; i < cli->argc; i++) {

    printf("Positional arg: %s\n", cli->argv[i]);
  }

  HTILS_TEST_RESULT();
}
