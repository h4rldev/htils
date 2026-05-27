#include <h2o/memory.h>

//
//
//

#include <htils/assert.h>
#include <htils/basictypes.h>

//
//
//

#include <h2otils/cookie.h>
#include <h2otils/darray.h>
#include <h2otils/string.h>

//
//
//

h2o_cookie_t *h2o_cookie_new(h2o_mem_pool_t *pool, const h2o_string *name,
                             const h2o_string *value) {
  h2o_cookie_t *cookie = h2o_mem_alloc_pool(pool, h2o_cookie_t, 1);
  cookie->name = h2o_string_dup(pool, name);
  cookie->value = h2o_string_dup(pool, value);
  cookie->domain = null;
  cookie->path = null;
  cookie->max_age = -1;
  cookie->same_site = INVALID;
  cookie->secure = false;
  cookie->http_only = false;

  return cookie;
}

h2o_cookie_t *h2o_cookie_from_string(h2o_mem_pool_t *pool,
                                     const h2o_string *str) {
  h2o_string **strings;
  h2o_da_new(pool, strings, 2);

  h2o_string_split((h2o_string *)str, ';', &strings, pool);
  for (u64 i = 0; i < h2o_da_len(strings); i++) {
    printf("String: %s\n", h2o_string_to_cstr(strings[i]));
  }
}
