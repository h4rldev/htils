#ifndef H2OTILS_COOKIE_H
#define H2OTILS_COOKIE_H

#include <h2o/memory.h>

//
//
//

#include <htils/basictypes.h>

//
//
//

#include <h2otils/string.h>
#include <h2otils/stringmap.h>

//
//
//

/**
 * @brief SameSite argument types.
 *
 * @param INVALID An invalid SameSite value, for initializing only, or to mark
 * SameSite as not being a part of the Cookie.
 * @param NONE The None SameSite value.
 * @param LAX The Lax SameSite value.
 * @param STRICT The Strict SameSite value.
 */
typedef enum h2o_same_site_args {
  INVALID = -1,
  NONE = 0,
  LAX = 1,
  STRICT = 2,
} h2o_same_site_args_t;

/**
 * @brief Cookie structure
 *
 * @param names The names of each cookie field.
 * @param values The values of each cookie field.
 * @param domain The domain of the cookie.
 * @param path The path of the cookie.
 * @param expires_str The expires string of the cookie.
 * @param same_site The SameSite value of the cookie.
 * @param expires The expires value of the cookie.
 * @param max_age The max-age value of the cookie.
 * @param secure The secure value of the cookie.
 * @param http_only The http-only value of the cookie.
 */
typedef struct h2o_cookie {
  h2o_stringmap_t *map;
  h2o_string *domain;
  h2o_string *path;
  h2o_string *expires_str;

  h2o_same_site_args_t same_site;

  i64 expires;
  i64 max_age;
  b32 secure;
  b32 http_only;
} h2o_cookie_t;

/**
 * @brief Cookie parameter types.
 *
 * @param SAME_SITE The SameSite parameter.
 * @param HTTP_ONLY The HttpOnly parameter.
 * @param PATH The Path parameter.
 * @param EXPIRES The Expires parameter.
 * @param MAX_AGE The Max-Age parameter.
 * @param DOMAIN The Domain parameter.
 * @param SECURE The Secure parameter.
 */
typedef enum h2o_cookie_param {
  SAME_SITE,
  HTTP_ONLY,
  PATH,
  EXPIRES,
  MAX_AGE,
  DOMAIN,
  SECURE,
} h2o_cookie_param_t;

//
//
//

/**
 * @brief Creates a new cookie with values.
 *
 * @details Allocates a new cookie with the given values to conform to the
 * Set-Cookie syntax.
 *
 * @param pool The memory pool to allocate the cookie from.
 * @param names The names of each cookie field.
 * @param values The values of each cookie field.
 * @param amount The amount of cookie keys and values within the cookie.
 *
 * @pre @c pool, @c names, and @c values must be valid and cannot be `null`.
 */
h2o_cookie_t *h2o_cookie_new(h2o_mem_pool_t *pool, const h2o_string **names,
                             const h2o_string **values, const u64 amount);

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

/**
 * @brief Appends a key-value pair to the cookie.
 *
 * @details Appends a key-value pair to the cookie, if the key already exists,
 * its updated, otherwise it will be created.
 *
 * @param cookie The cookie to append to.
 * @param key The key to append.
 * @param value The value to append.
 *
 * @pre @c cookie, @c key, and @c value must be valid and cannot be `null`.
 */
void h2o_cookie_append(h2o_cookie_t *cookie, const h2o_string *key,
                       const h2o_string *value);

//
//
//

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

/**
 * @brief Gets a value from the cookie.
 *
 * @details Gets a value from the cookie, if the key doesn't exist, it will
 * return null.
 *
 * @param cookie The cookie to get the value from.
 * @param key The key to get the value from.
 *
 * @pre @c cookie and @c key must be valid and cannot be `null`.
 *
 * @return A pointer to the value, or null if it doesn't exist.
 */
h2o_string *h2o_cookie_get_value(h2o_cookie_t *cookie, const h2o_string *key);

#endif // H2OTILS_COOKIE_H
