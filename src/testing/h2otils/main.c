#include <h2o/memory.h>

#include <htils/basictypes.h>

#include <h2otils/cookie.h>
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

H2OTILS_TEST(cookie_new) { return H2OTILS_TEST_PASS; }

int main(void) {
  u32 test_count = 0;
  u32 failures = 0;
  u32 skips = 0;

  H2OTILS_TEST_RUN(h2o_string_new);
  H2OTILS_TEST_RUN(h2o_string_from_cstr);

  H2OTILS_TEST_RUN(cookie_new);

  H2OTILS_TEST_RESULT();
}
