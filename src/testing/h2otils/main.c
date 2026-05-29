#include <h2o/memory.h>

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

#include <h2otils/cookie.h>
#include <h2otils/darray.h>
#include <h2otils/string.h>
#include <h2otils/test.h>

H2OTILS_TEST(h2o_string_new) {
  h2o_string *str = h2o_string_new(pool, 4);
  memcpy(str->base, "meow", 4);

  H2OTILS_TEST_ASSERT(
      str, "String is null, something went wrong on making the string.");
  H2OTILS_TEST_ASSERT(
      str->len == 4,
      "String length is not 4, something went wrong on making the string.");
  H2OTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                      "String is not 'meow', when it was expected.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_from_cstr) {
  h2o_string *str = h2o_string_from_cstr(pool, "meow");

  H2OTILS_TEST_ASSERT(
      str, "String is null, something went wrong on making the string.");
  H2OTILS_TEST_ASSERT(
      str->len == 4,
      "String length is not 4, something went wrong on making the string.");
  H2OTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                      "String is not 'meow', when it was expected.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_from_string) {
  arena_t *arena = arena_new(MiB(1), KiB(128));

  h2o_string *str = h2o_string_from_string(pool, HTILS_STR("meow"));
  H2OTILS_TEST_ASSERT(
      str, "String is null, something went wrong on making the string.");
  H2OTILS_TEST_ASSERT(
      str->len == 4,
      "String length is not 4, something went wrong on making the string.");
  H2OTILS_TEST_ASSERT(
      str->base,
      "String base is null, something went wrong on making the string.");
  H2OTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                      "String is not 'meow', when it was expected.");

  arena_free(arena);
  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_dup) {
  h2o_string *str = h2o_string_from_cstr(pool, "meow");

  H2OTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                      "String is not 'meow'.");

  h2o_string *dup = h2o_string_dup(pool, str);
  H2OTILS_TEST_ASSERT(dup, "Dup string is null, something went wrong.");
  H2OTILS_TEST_ASSERT(dup->len == 4, "Dup string length is not 4.");
  H2OTILS_TEST_ASSERT(dup->base, "Dup string base is null.");
  H2OTILS_TEST_ASSERT(memcmp(dup->base, "meow", 4) == 0,
                      "Dup string is not 'meow'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_slice_from_cstr) {
  h2o_string_slice slice = h2o_string_slice_from_cstr("meow", 4);

  H2OTILS_TEST_ASSERT(slice.base, "Slice base is null.");
  H2OTILS_TEST_ASSERT(slice.len == 4, "Slice length is not 4.");
  H2OTILS_TEST_ASSERT(memcmp(slice.base, "meow", 4) == 0,
                      "Slice is not 'meow'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_slice_from_h2o_string) {
  h2o_string *str = h2o_string_from_cstr(pool, "meow");

  H2OTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                      "String is not 'meow'.");

  h2o_string_slice slice = h2o_string_slice_from_h2o_string(str);

  H2OTILS_TEST_ASSERT(slice.base, "Slice base is null.");
  H2OTILS_TEST_ASSERT(slice.len == 4, "Slice length is not 4.");
  H2OTILS_TEST_ASSERT(memcmp(slice.base, "meow", 4) == 0,
                      "Slice is not 'meow'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_slice_from_string) {
  arena_t *arena = arena_new(MiB(1), KiB(128));

  string *str = string_from_cstr(arena, "meow");

  H2OTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                      "String is not 'meow'.");

  h2o_string_slice slice = h2o_string_slice_from_string(str);

  H2OTILS_TEST_ASSERT(slice.base, "Slice base is null.");
  H2OTILS_TEST_ASSERT(slice.len == 4, "Slice length is not 4.");
  H2OTILS_TEST_ASSERT(memcmp(slice.base, "meow", 4) == 0,
                      "Slice is not 'meow'.");

  arena_free(arena);
  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_to_cstr) {
  h2o_string *str = h2o_string_from_cstr(pool, "meow");

  H2OTILS_TEST_ASSERT(str, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str->base, "meow", 4) == 0,
                      "String is not 'meow'.");

  cstr *cstr = h2o_string_to_cstr(str);
  H2OTILS_TEST_ASSERT(cstr, "C-String is null, something went wrong.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_concat) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "meow");
  h2o_string *str2 = h2o_string_from_cstr(pool, "meow2");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str2, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str2->len == 5, "String length is not 5.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                      "String is not 'meow'.");
  H2OTILS_TEST_ASSERT(memcmp(str2->base, "meow2", 5) == 0,
                      "String is not 'meow2'.");

  u64 concat_len = h2o_string_concat(pool, str1, str2);
  H2OTILS_TEST_ASSERT(concat_len == 9, "Concatenated string length is not 9.");
  H2OTILS_TEST_ASSERT(str1,
                      "Concatenated string is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->base, "Concatenated string base is null.");
  H2OTILS_TEST_ASSERT(str1->len == 9, "Concatenated string length is not 9.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meowmeow2", 9) == 0,
                      "Concated string is not 'meowmeow2'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_concatb) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "meow");
  h2o_string *str2 = h2o_string_from_cstr(pool, "meow2");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str2, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(str2->base, "String base is null.");
  H2OTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str2->len == 5, "String length is not 5.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                      "String is not 'meow'.");
  H2OTILS_TEST_ASSERT(memcmp(str2->base, "meow2", 5) == 0,
                      "String is not 'meow2'.");

  u64 concat_len = h2o_string_concatb(pool, str1, str2, 2);
  H2OTILS_TEST_ASSERT(concat_len == 6, "Concatenated string length is not 6.");
  H2OTILS_TEST_ASSERT(str1,
                      "Concatenated string is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->base, "Concatenated string base is null.");
  H2OTILS_TEST_ASSERT(str1->len == 6, "Concatenated string length is not 6.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meowme", 6) == 0,
                      "Concated string is not 'meowme'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_concatf) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "meow");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                      "Concated string is not 'meow'.");

  u64 concat_len = h2o_string_concatf(pool, str1, "%s123", "meow2");
  H2OTILS_TEST_ASSERT(concat_len == 12,
                      "Concatenated string length is not 12.");
  H2OTILS_TEST_ASSERT(str1,
                      "Concatenated string is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->base, "Concatenated string base is null.");
  H2OTILS_TEST_ASSERT(str1->len == 12, "Concatenated string length is not 12.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meowmeow2123", 12) == 0,
                      "Concated string is not 'meowmeow2123'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_stringcmp) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "meow");
  h2o_string *str2 = h2o_string_dup(pool, str1);
  h2o_string *str3 = h2o_string_from_cstr(pool, "meow2");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str2, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str3, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str2->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str3->len == 5, "String length is not 5.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                      "String is not 'meow'.");
  H2OTILS_TEST_ASSERT(memcmp(str2->base, "meow", 4) == 0,
                      "String is not 'meow'.");
  H2OTILS_TEST_ASSERT(memcmp(str3->base, "meow2", 5) == 0,
                      "String is not 'meow2'.");

  b32 cmp = h2o_stringcmp(str1, str2);
  H2OTILS_TEST_ASSERT(cmp == true, "Strings are not equal.");

  b32 cmp2 = h2o_stringcmp(str1, str3);
  H2OTILS_TEST_ASSERT(cmp2 == false, "Strings are equal when they shouldn't.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_stringcmpb) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "meow");
  h2o_string *str2 = h2o_string_dup(pool, str1);
  h2o_string *str3 = h2o_string_from_cstr(pool, "me2");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str2, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str3, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str2->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str3->len == 3, "String length is not 5.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                      "String is not 'meow'.");
  H2OTILS_TEST_ASSERT(memcmp(str2->base, "meow", 4) == 0,
                      "String is not 'meow'.");
  H2OTILS_TEST_ASSERT(memcmp(str3->base, "me2", 3) == 0,
                      "String is not 'me2'.");

  b32 cmp = h2o_stringcmpb(str1, str2, 4);
  H2OTILS_TEST_ASSERT(cmp == true, "Strings are not equal.");

  b32 cmp2 = h2o_stringcmpb(str1, str3, 2);
  H2OTILS_TEST_ASSERT(cmp2 == true, "Strings are not equal.");

  b32 cmp3 = h2o_stringcmpb(str1, str3, 3);
  H2OTILS_TEST_ASSERT(cmp3 == false, "Strings are equal when they shouldn't.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_trim) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "  meow  ");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 8, "String length is not 8.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "  meow  ", 8) == 0,
                      "String is not '  meow  '.");

  h2o_string_trim(str1);

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                      "String is not 'meow'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_trim_left) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "  meow  ");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 8, "String length is not 8.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "  meow  ", 8) == 0,
                      "String is not '  meow  '.");

  h2o_string_trim_left(str1);

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 6, "String length is not 6.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow  ", 6) == 0,
                      "String is not 'meow  '.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_trim_right) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "  meow  ");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 8, "String length is not 8.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "  meow  ", 8) == 0,
                      "String is not '  meow  '.");

  h2o_string_trim_right(str1);

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 6, "String length is not 6.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "  meow", 6) == 0,
                      "String is not '  meow'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_findc) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "meow");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow", 4) == 0,
                      "String is not 'meow'.");

  i64 index = h2o_string_findc(str1, 'e');
  H2OTILS_TEST_ASSERT(index == 1, "String wasn't found.");

  index = h2o_string_findc(str1, 'm');
  H2OTILS_TEST_ASSERT(index == 0, "String wasn't found.");

  index = h2o_string_findc(str1, 'q');
  H2OTILS_TEST_ASSERT(index == -1, "String was found.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_string_find_sstr) {
  h2o_string *str1 = h2o_string_from_cstr(pool, "meow meow2 meow3");

  H2OTILS_TEST_ASSERT(str1, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str1->len == 16, "String length is not 16.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow meow2 meow3", 16) == 0,
                      "String is not 'meow meow2 meow3'.");

  h2o_string *str2 = h2o_string_from_cstr(pool, "meow");

  H2OTILS_TEST_ASSERT(str2, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str2->len == 4, "String length is not 4.");
  H2OTILS_TEST_ASSERT(str2->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str2->base, "meow", 4) == 0,
                      "String is not 'meow'.");

  h2o_string *str3 = h2o_string_from_cstr(pool, "meow2");

  H2OTILS_TEST_ASSERT(str3, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str3->len == 5, "String length is not 5.");
  H2OTILS_TEST_ASSERT(str3->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str3->base, "meow2", 5) == 0,
                      "String is not 'meow2'.");

  h2o_string *str4 = h2o_string_from_cstr(pool, "meow3");

  H2OTILS_TEST_ASSERT(str4, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str4->len == 5, "String length is not 5.");
  H2OTILS_TEST_ASSERT(str4->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str4->base, "meow3", 5) == 0,
                      "String is not 'meow3'.");

  h2o_string *str5 = h2o_string_from_cstr(pool, "meow4");

  H2OTILS_TEST_ASSERT(str5, "String is null, something went wrong.");
  H2OTILS_TEST_ASSERT(str5->len == 5, "String length is not 5.");
  H2OTILS_TEST_ASSERT(str5->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str5->base, "meow4", 5) == 0,
                      "String is not 'meow4'.");

  i64 index = h2o_string_find_sstr(str1, str2);
  H2OTILS_TEST_ASSERT(index == 0, "String wasn't found.");

  index = h2o_string_find_sstr(str1, str3);
  H2OTILS_TEST_ASSERT(index == 5, "String wasn't found.");

  index = h2o_string_find_sstr(str1, str4);
  H2OTILS_TEST_ASSERT(index == 11, "String wasn't found.");

  index = h2o_string_find_sstr(str1, str5);
  H2OTILS_TEST_ASSERT(index == -1, "String was found.");

  return H2OTILS_TEST_PASS;
}

//
//
//

H2OTILS_TEST(h2o_da_new) {
  h2o_string **strs = {0};
  h2o_da_new(pool, strs, 1);

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 1,
                      "Dynamic array capacity is not 1.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 0, "Dynamic array length is not 0.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_da_append) {
  h2o_string **strs = {0};
  h2o_da_new(pool, strs, 1);

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 1,
                      "Dynamic array capacity is not 1.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 0, "Dynamic array length is not 0.");

  h2o_da_append(pool, strs, H2OTILS_STR("meow"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow2"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow3"));

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 3,
                      "Dynamic array capacity is not 3.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 3, "Dynamic array length is not 3.");

  for (u64 i = 0; i < h2o_da_len(strs); i++) {
    H2OTILS_TEST_ASSERT(strs[i], "Dynamic array item is null.");
    H2OTILS_TEST_ASSERT(strs[i]->base, "Dynamic array item base is null.");
    H2OTILS_TEST_ASSERT(strs[i]->len >= 4 && strs[i]->len <= 5,
                        "String length is not between 4 and 5.");
    H2OTILS_TEST_ASSERT(memcmp(strs[i]->base, "meow", 4) == 0,
                        "Dynamic array item doesn't contain 'meow'.");
  }

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_da_pop) {
  h2o_string **strs = {0};
  h2o_da_new(pool, strs, 1);

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 1,
                      "Dynamic array capacity is not 1.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 0, "Dynamic array length is not 0.");

  h2o_da_append(pool, strs, H2OTILS_STR("meow"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow2"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow3"));

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 3,
                      "Dynamic array capacity is not 3.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 3, "Dynamic array length is not 3.");

  h2o_da_pop(strs);
  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 3,
                      "Dynamic array capacity is not 3.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 2, "Dynamic array length is not 2.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_da_last) {
  h2o_string **strs = {0};
  h2o_da_new(pool, strs, 1);

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 1,
                      "Dynamic array capacity is not 1.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 0, "Dynamic array length is not 0.");

  h2o_da_append(pool, strs, H2OTILS_STR("meow"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow2"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow3"));

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 3,
                      "Dynamic array capacity is not 3.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 3, "Dynamic array length is not 3.");

  h2o_string *last = h2o_da_last(strs);
  H2OTILS_TEST_ASSERT(last, "Last string is null.");
  H2OTILS_TEST_ASSERT(last->base, "Last string base is null.");
  H2OTILS_TEST_ASSERT(last->len == 5, "Last string length is not 4.");
  H2OTILS_TEST_ASSERT(memcmp(last->base, "meow3", 5) == 0,
                      "Last string is not 'meow3'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_da_clear) {
  h2o_string **strs = {0};
  h2o_da_new(pool, strs, 1);

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 0, "Dynamic array length is not 0.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 1,
                      "Dynamic array capacity is not 1.");

  h2o_da_append(pool, strs, H2OTILS_STR("meow"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow2"));
  h2o_da_append(pool, strs, H2OTILS_STR("meow3"));

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 3, "Dynamic array length is not 3.");
  H2OTILS_TEST_ASSERT(h2o_da_cap(strs) == 3,
                      "Dynamic array capacity is not 3.");

  h2o_da_clear(strs);

  H2OTILS_TEST_ASSERT(strs, "Dynamic array is null.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 0, "Dynamic array length is not 0.");

  return H2OTILS_TEST_PASS;
}

//
//
//

H2OTILS_TEST(h2o_string_split) {
  h2o_string **strs = {0};
  h2o_da_new(pool, strs, 1);

  h2o_string *str1 = h2o_string_from_cstr(pool, "meow\nmeow\nmeow\n");
  H2OTILS_TEST_ASSERT(str1, "String is null.");
  H2OTILS_TEST_ASSERT(str1->len == 15, "String length is not 15.");
  H2OTILS_TEST_ASSERT(str1->base, "String base is null.");
  H2OTILS_TEST_ASSERT(memcmp(str1->base, "meow\nmeow\nmeow\n", 15) == 0,
                      "String is not 'meow\\nmeow\\nmeow\\n'.");

  u64 amount = h2o_string_split(str1, '\n', &strs, pool);
  H2OTILS_TEST_ASSERT(amount == h2o_da_len(strs),
                      "Strings were not split properly.");
  H2OTILS_TEST_ASSERT(h2o_da_len(strs) == 3,
                      "Strings were not split properly.");

  for (u64 i = 0; i < amount; i++) {
    H2OTILS_TEST_ASSERT(strs[i], "String is null.");
    H2OTILS_TEST_ASSERT(strs[i]->len == 4, "String length is not 4.");
    H2OTILS_TEST_ASSERT(strs[i]->base, "String base is null.");
    H2OTILS_TEST_ASSERT(memcmp(strs[i]->base, "meow", 4) == 0,
                        "String is not 'meow'.");
  }

  return H2OTILS_TEST_PASS;
}

//
//
//

H2OTILS_TEST(cookie_new) {
  h2o_cookie_t *cookie =
      h2o_cookie_new(pool, H2OTILS_STR("meow"), H2OTILS_STR("meow"));

  H2OTILS_TEST_ASSERT(cookie, "Cookie is null.");
  H2OTILS_TEST_ASSERT(cookie->domain == null, "Cookie domain is not null.");
  H2OTILS_TEST_ASSERT(cookie->path == null, "Cookie path is not null.");
  H2OTILS_TEST_ASSERT(cookie->max_age == -1, "Cookie max_age is not -1.");
  H2OTILS_TEST_ASSERT(cookie->same_site == INVALID,
                      "Cookie SameSite is not -1.");
  H2OTILS_TEST_ASSERT(cookie->secure == false, "Cookie secure is not false.");
  H2OTILS_TEST_ASSERT(cookie->http_only == false,
                      "Cookie http_only is not false.");

  H2OTILS_TEST_ASSERT(cookie->name, "Cookie name is null.");
  H2OTILS_TEST_ASSERT(cookie->name->len == 4, "Cookie name length is not 4.");
  H2OTILS_TEST_ASSERT(cookie->name->base, "Cookie name base is null.");
  H2OTILS_TEST_ASSERT(memcmp(cookie->name->base, "meow", 4) == 0,
                      "Cookie name is not 'meow'.");

  H2OTILS_TEST_ASSERT(cookie->value, "Cookie value is null.");
  H2OTILS_TEST_ASSERT(cookie->value->len == 4, "Cookie value length is not 4.");
  H2OTILS_TEST_ASSERT(cookie->value->base, "Cookie value base is null.");
  H2OTILS_TEST_ASSERT(memcmp(cookie->value->base, "meow", 4) == 0,
                      "Cookie value is not 'meow'.");

  return H2OTILS_TEST_PASS;
}

H2OTILS_TEST(h2o_cookie_from_string) {
  h2o_string *str = h2o_string_from_cstr(
      pool, "meow=meow2; path=/; domain=h4rl.dev; expires=Mon, 1 Jun 2026 "
            "09:22:50 GMT; secure; httponly");

  H2OTILS_TEST_ASSERT(str, "String is null.");
  H2OTILS_TEST_ASSERT(str->len == 91, "String length is not 91.");
  H2OTILS_TEST_ASSERT(str->base, "String base is null.");
  H2OTILS_TEST_ASSERT(
      memcmp(str->base,
             "meow=meow2; path=/; domain=h4rl.dev; expires=Mon, 1 Jun 2026 "
             "09:22:50 GMT; secure; httponly",
             91) == 0,
      "String is not 'meow=meow2; path=/; domain=h4rl.dev; expires=Mon, 1 Jun "
      "2026 09:22:50 GMT; secure; httponly'.");

  h2o_cookie_t *cookie = h2o_cookie_from_string(pool, str);
  H2OTILS_TEST_ASSERT(cookie, "Cookie is null.");

  H2OTILS_TEST_ASSERT(cookie->name, "Cookie name is null.");
  H2OTILS_TEST_ASSERT(cookie->name->len == 4, "Cookie name length is not 4.");
  H2OTILS_TEST_ASSERT(cookie->name->base, "Cookie name base is null.");
  H2OTILS_TEST_ASSERT(memcmp(cookie->name->base, "meow", 4) == 0,
                      "Cookie name base is not 'meow'.");

  H2OTILS_TEST_ASSERT(cookie->value, "Cookie value is null.");
  H2OTILS_TEST_ASSERT(cookie->value->len == 5, "Cookie value length is not 4.");
  H2OTILS_TEST_ASSERT(cookie->value->base, "Cookie value base is null.");
  H2OTILS_TEST_ASSERT(memcmp(cookie->value->base, "meow2", 5) == 0,
                      "Cookie value base is not 'meow2'.");

  H2OTILS_TEST_ASSERT(cookie->domain, "Cookie domain is null.");
  H2OTILS_TEST_ASSERT(cookie->domain->len == 8,
                      "Cookie domain length is not 8.");
  H2OTILS_TEST_ASSERT(cookie->domain->base, "Cookie domain base is null.");
  H2OTILS_TEST_ASSERT(memcmp(cookie->domain->base, "h4rl.dev", 8) == 0,
                      "Cookie domain isn't 'h4rl.dev'.");

  H2OTILS_TEST_ASSERT(cookie->path, "Cookie path is null.");
  H2OTILS_TEST_ASSERT(cookie->path->len == 1, "Cookie path length is not 1.");
  H2OTILS_TEST_ASSERT(cookie->path->base, "Cookie path base is null.");
  H2OTILS_TEST_ASSERT(memcmp(cookie->path->base, "/", 1) == 0,
                      "Cookie path isn't '/'.");
  H2OTILS_TEST_ASSERT(cookie->max_age == -1, "Cookie max_age is not -1.");
  H2OTILS_TEST_ASSERT(cookie->same_site == INVALID,
                      "Cookie SameSite is not INVALID.");
  H2OTILS_TEST_ASSERT(cookie->secure == true, "Cookie secure is not true.");
  H2OTILS_TEST_ASSERT(cookie->http_only == true,
                      "Cookie http_only is not true.");
  H2OTILS_TEST_ASSERT(cookie->expires != -1, "Cookie expires is -1.");
  H2OTILS_TEST_ASSERT(cookie->expires_str, "Cookie expires str is null.");
  H2OTILS_TEST_ASSERT(cookie->expires_str->len == 28,
                      "Cookie expires str length is not 28.");
  H2OTILS_TEST_ASSERT(cookie->expires_str->base,
                      "Cookie expires str base is null.");
  H2OTILS_TEST_ASSERT(
      memcmp(cookie->expires_str->base, "mon, 1 jun 2026 09:22:50 gmt", 28) ==
          0,
      "Cookie expires str is not 'mon, 1 Jun 2026 09:22:50 gmt'.");

  return H2OTILS_TEST_PASS;
}

int main(void) {
  u32 test_count = 0;
  u32 failures = 0;
  u32 skips = 0;

  H2OTILS_TEST_RUN(h2o_string_new);
  H2OTILS_TEST_RUN(h2o_string_from_cstr);
  H2OTILS_TEST_RUN(h2o_string_from_string);
  H2OTILS_TEST_RUN(h2o_string_dup);
  H2OTILS_TEST_RUN(h2o_string_slice_from_h2o_string);
  H2OTILS_TEST_RUN(h2o_string_slice_from_cstr);
  H2OTILS_TEST_RUN(h2o_string_slice_from_string);
  H2OTILS_TEST_RUN(h2o_string_to_cstr);
  H2OTILS_TEST_RUN(h2o_string_concat);
  H2OTILS_TEST_RUN(h2o_string_concatb);
  H2OTILS_TEST_RUN(h2o_string_concatf);
  H2OTILS_TEST_RUN(h2o_stringcmp);
  H2OTILS_TEST_RUN(h2o_stringcmpb);
  H2OTILS_TEST_RUN(h2o_string_trim);
  H2OTILS_TEST_RUN(h2o_string_trim_left);
  H2OTILS_TEST_RUN(h2o_string_trim_right);
  H2OTILS_TEST_RUN(h2o_string_findc);
  H2OTILS_TEST_RUN(h2o_string_find_sstr);

  H2OTILS_TEST_RUN(h2o_da_new);
  H2OTILS_TEST_RUN(h2o_da_append);
  H2OTILS_TEST_RUN(h2o_da_pop);
  H2OTILS_TEST_RUN(h2o_da_last);
  H2OTILS_TEST_RUN(h2o_da_clear);

  H2OTILS_TEST_RUN(h2o_string_split);

  H2OTILS_TEST_RUN(cookie_new);
  H2OTILS_TEST_RUN(h2o_cookie_from_string);

  H2OTILS_TEST_RESULT();
}
