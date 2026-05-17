#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/darray.h>
#include <htils/file.h>
#include <htils/hashmap.h>
#include <htils/path.h>
#include <htils/string.h>
#include <htils/test.h>

HTILS_TEST(file_write) {
  string *path_to_write = string_from_cstr(arena, "test.txt");
  string *contents = string_from_cstr(arena, "meow\nmeow\nmeow\n");

  u64 written = write_to_file(path_to_write, contents);
  HTILS_TEST_ASSERT(
      written == contents->len,
      "Failed to write to file properly, or write_to_file was misimplemented.");
  HTILS_TEST_ASSERT(does_path_exist(path_to_write), "Failed to write to file.");

  return HTILS_TEST_PASS;
}

int main(void) {
  arena_t *arena = arena_new(MiB(2), KiB(128));
  u32 failures = 0;

  HTILS_TEST_RUN(file_write, failures);

  if (failures > 0) {
    fprintf(stderr, "\n%u Tests FAILED.\n", failures);
    return 1;
  }

  fprintf(stderr, "\nAll tests PASSED.\n");
  return 0;
}
