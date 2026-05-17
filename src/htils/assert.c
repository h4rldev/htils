#include <stdio.h>
#include <stdlib.h>

#include <htils/assert.h>
#include <htils/basictypes.h>

//
//
//

_Noreturn void __htils_assert_fail(const cstr *expr_str, const cstr *executable,
                                   const cstr *file, u32 line,
                                   const cstr *func) {
  fprintf(stderr, "%s: %s:%i: %s: Assertion `%s` failed.\n", executable, file,
          line, func, expr_str);
  exit(1);
}
