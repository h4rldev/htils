#ifndef H2OTILS_COOKIE_H
#define H2OTILS_COOKIE_H

#include <h2o/memory.h>

//
//
//

#include <h2otils/string.h>
#include <htils/basictypes.h>

//
//
//

typedef enum h2o_same_site_args {
  INVALID = -1,
  NONE = 0,
  LAX = 1,
  STRICT = 2,
} h2o_same_site_args_t;

typedef struct h2o_cookie {
  h2o_string *name;
  h2o_string *value;
  h2o_string *domain;
  h2o_string *path;
  h2o_string *expires_str;

  i64 expires;
  i64 max_age;
  h2o_same_site_args_t same_site;
  b32 secure;
  b32 http_only;
} h2o_cookie_t;

typedef enum h2o_cookie_param {
  SAME_SITE,
  HTTP_ONLY,
  PATH,
  EXPIRES,
  MAX_AGE,
  DOMAIN,
  SECURE,
} h2o_cookie_param_t;

/**
 * @brief Creates a new cookie with values.
 *
 * @details Allocates a new cookie with the given values to conform to the
 * Set-Cookie syntax.
 *
 * @param pool The memory pool to allocate the cookie from.
 * @param name The name of the cookie.
 * @param value The value of the cookie.
 *
 * @pre @c pool, @c name, and @c value must be valid and cannot be `null`.
 */
h2o_cookie_t *h2o_cookie_new(h2o_mem_pool_t *pool, const h2o_string *name,
                             const h2o_string *value);

/**
 * @brief Creates a new cookie from a string.
 *
 * @details Parses the string and creates a new cookie from it.
 *
 * @param pool The memory pool to allocate the cookie from.
 * @param str The string to parse.
 *
 * @pre @c pool and @c str must be valid and cannot be `null`.
 *
 * @return A pointer to the new cookie.
 */
h2o_cookie_t *h2o_cookie_from_string(h2o_mem_pool_t *pool,
                                     const h2o_string *str);

/**
 * @brief Converts a cookie to a h2o_string.
 *
 * @details Converts the cookie to a h2o_string, according to the Set-Cookie
 * syntax.
 *
 * @param pool The memory pool to allocate the string from.
 * @param cookie The cookie to convert.
 *
 * @pre @c pool and @c cookie must be valid and cannot be `null`.
 *
 * @return A pointer to the new h2o_string.
 */
h2o_string *h2o_cookie_to_string(h2o_mem_pool_t *pool, h2o_cookie_t *cookie);

//
//
//

/**
 * @brief Adds a parameter to the cookie.
 *
 * @details Checks the parameter, and appends the modifier according to
 * Set-Cookie syntax.
 *
 * @param pool The memory pool to allocate the cookie from.
 * @param cookie The cookie to add the parameter to.
 * @param param The parameter to add.
 * @param ... The value of the parameter, as provided by variadic args.
 *
 * @pre
 * - @c pool, @c cookie, and @c val must be valid and cannot be `null`.
 * - @c param must be a valid parameter.
 */
void h2o_cookie_add_param(h2o_mem_pool_t *pool, h2o_cookie_t *cookie,
                          h2o_cookie_param_t param, ...);

#endif // H2OTILS_COOKIE_H
