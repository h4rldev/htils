#include <ctype.h>
#include <stdarg.h>
#include <time.h>

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
static inline void to_lower(h2o_string *str) {
  for (u64 i = 0; i < str->len; i++)
    str->base[i] = tolower(str->base[i]);
}

static inline i32 parse_3let_month(const cstr *month_cstr) {
  if (!month_cstr || !month_cstr[0] || !month_cstr[1] || !month_cstr[2])
    return -1;

  char a = tolower(month_cstr[0]), b = tolower(month_cstr[1]),
       c = tolower(month_cstr[2]);

  if (a == 'j') {
    if (b == 'a' && c == 'n')
      return 0;
    else if (b == 'u') {
      if (c == 'n')
        return 5;
      else if (c == 'l')
        return 6;
    }
  }

  else if (a == 'f' && b == 'e' && c == 'b')
    return 1;

  else if (a == 'a') {
    if (b == 'p' && c == 'r')
      return 3;
    else if (b == 'u' && c == 'g')
      return 7;
  }

  else if (a == 'm' && b == 'a') {
    if (c == 'r')
      return 2;
    else if (c == 'y')
      return 4;
  }

  else if (a == 's' && b == 'e' && c == 'p')
    return 8;
  else if (a == 'o' && b == 'c' && c == 't')
    return 9;
  else if (a == 'n' && b == 'o' && c == 'v')
    return 10;
  else if (a == 'd' && b == 'e' && c == 'c')
    return 11;

  return -1;
}

static inline i32 parse_str_to_num(const cstr *str, i32 max_digits) {
  i32 res = 0;
  for (i32 i = 0; i < max_digits; i++) {
    char c = str[i];
    if (!isdigit(c))
      break;

    if (c >= '0' && c <= '9')
      res = res * 10 + (c - '0');
    else
      break;
  }

  return res;
}

//
//
//

static b32 parse_imf_fixdate(const cstr *date_str, time_t *result) {
  struct tm tm = {0};
  i32 month = 0, day = 0, year = 0, hour = 0, min = 0, sec = 0;

  if (strlen(date_str) < 29) {
    fprintf(stderr, "Date string is too short.\n");
    return false;
  }

  if (date_str[3] != ',' && date_str[4] != ' ') {
    fprintf(stderr, "Date string is invalid.\n");
    return false;
  }

  day = parse_str_to_num(&date_str[5], 2);
  month = parse_3let_month(&date_str[8]);
  year = parse_str_to_num(&date_str[12], 4);
  hour = parse_str_to_num(&date_str[17], 2);
  min = parse_str_to_num(&date_str[20], 2);
  sec = parse_str_to_num(&date_str[23], 2);

  if (date_str[26] != 'G' && date_str[27] != 'M' && date_str[28] != 'T' &&
      date_str[26] != 'U' && date_str[27] != 'T' && date_str[28] != 'C') {
    fprintf(stderr, "Date string is invalid, missing 'GMT' or 'UTC'.\n");
    fprintf(stderr, "Invalid Date string: %s\n", date_str);
    fprintf(stderr, "Expected 'G' or 'U' at position 26, found %c\n",
            date_str[26]);
    fprintf(stderr, "Expected 'M' or 'T' at position 27, found %c\n",
            date_str[27]);
    fprintf(stderr, "Expected 'T' pr 'C' at position 28, found %c\n",
            date_str[28]);
    return false;
  }

  if (month >= 0 && day >= 1 && day <= 31 && year >= 1970 && hour >= 0 &&
      hour < 24 && min >= 0 && min < 60 && sec >= 0 && sec < 60) {
    tm.tm_mday = day;
    tm.tm_mon = month;
    tm.tm_year = year - 1900;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = 0;

    *result = timegm(&tm);
    return (*result != -1);
  }

  return false;
}

static b32 parse_http_date(const cstr *date_cstr, time_t *result) {
  if (date_cstr[3] == ',' && date_cstr[4] == ' ')
    return parse_imf_fixdate(date_cstr, result);

  fprintf(stderr, "Unknown date format.\n");
  return false;
}

static b32 validate_expires(const cstr *expires_str, i64 *expires) {
  time_t expires_time, current_time;

  if (!parse_http_date(expires_str, &expires_time)) {
    fprintf(stderr, "Invalid expires cookie value.\n");
    return false;
  }

  current_time = time(null);
  if (expires_time < current_time) {
    fprintf(stderr, "Cookie expires is in the past. %lu < %lu\n", expires_time,
            current_time);
    return false;
  }

  *expires = (i64)expires_time;
  return true;
}

//
//
//

h2o_cookie_t *h2o_cookie_new(h2o_mem_pool_t *pool, const h2o_string **names,
                             const h2o_string **values, const u64 amount) {
  h2o_cookie_t *cookie = h2o_mem_alloc_pool(pool, h2o_cookie_t, 1);
  *cookie = (h2o_cookie_t){0};
  h2o_stringmap_t *map = h2o_sm_new(pool, 1);

  for (u64 i = 0; i < amount; i++) {
    if (!names[i] || !values[i]) {
      fprintf(stderr, "Invalid cookie name or value.\n");
      return null;
    }

    h2o_sm_insert(map, names[i], values[i]);
  }

  cookie->map = map;
  cookie->max_age = -1;
  cookie->expires = -1;
  cookie->same_site = INVALID;

  return cookie;
}

typedef struct known_attr {
  cstr *name;
  u64 name_len;
} known_attr_t;

h2o_cookie_t *h2o_cookie_from_string(h2o_mem_pool_t *pool,
                                     const h2o_string *str) {
  h2o_string **strings;
  h2o_da_new(pool, strings, 2);

  if (h2o_string_split((h2o_string *)str, ';', &strings, pool) == 0) {
    fprintf(stderr, "Failed to split cookie string.\n");
    return null;
  }

  for (u64 i = 0; i < h2o_da_len(strings); i++)
    h2o_string_trim(strings[i]);

  h2o_cookie_t *cookie = h2o_mem_alloc_pool(pool, h2o_cookie_t, 1);
  *cookie = (h2o_cookie_t){0};
  cookie->same_site = INVALID;
  cookie->max_age = -1;
  cookie->map = h2o_sm_new(pool, 1);

  static const known_attr_t known_attrs[7] = {
      {.name = "path", .name_len = 4},     {.name = "domain", .name_len = 6},
      {.name = "max-age", .name_len = 7},  {.name = "samesite", .name_len = 8},
      {.name = "expires", .name_len = 7},  {.name = "secure", .name_len = 6},
      {.name = "httponly", .name_len = 8},
  };
  static const u64 known_attrs_len = 7;

  b32 is_attr_only = true;
  u64 first_attr_idx = h2o_da_len(strings);

  for (u64 i = 0; i < h2o_da_len(strings); i++) {
    i64 idx = h2o_string_findc(strings[i], '=');
    if (idx == -1) {
      u64 str_len = strings[i]->len;
      b32 is_attr = false;

      for (u64 j = 0; j < known_attrs_len; j++) {
        u64 attr_len = known_attrs[j].name_len;
        if (str_len == attr_len &&
            strncasecmp(strings[i]->base, known_attrs[j].name, attr_len) == 0) {
          is_attr = true;
          break;
        }
      }

      if (is_attr) {
        if (i < first_attr_idx)
          first_attr_idx = i;
        continue;
      } else {
        fprintf(stderr,
                "Malformed cookie segment: missing '=' at position %lu\n", i);
        return null;
      }
    }

    if (idx >= (i64)strings[i]->len) {
      fprintf(
          stderr,
          "Malformed cookie segment: '=' at end of string at position %lu\n",
          i);
      return null;
    }

    h2o_string_slice key_slice = (h2o_string_slice){
        .base = strings[i]->base,
        .len = idx,
    };

    b32 is_attr = false;
    for (u64 j = 0; j < known_attrs_len; j++) {
      u64 attr_len = known_attrs[j].name_len;
      if (key_slice.len == attr_len &&
          strncasecmp(key_slice.base, known_attrs[j].name, attr_len) == 0) {
        is_attr = true;
        if (i < first_attr_idx)
          first_attr_idx = i;
        break;
      }
    }

    if (is_attr)
      continue;

    is_attr_only = false;

    h2o_string_slice name_slice = (h2o_string_slice){
        .base = strings[i]->base,
        .len = idx,
    };
    h2o_string_slice value_slice = (h2o_string_slice){
        .base = strings[i]->base + idx + 1,
        .len = strings[i]->len - idx - 1,
    };

    h2o_string *name = h2o_string_dup(pool, &name_slice);
    h2o_string *value = h2o_string_dup(pool, &value_slice);

    h2o_sm_insert(cookie->map, name, value);
  }

  if (is_attr_only && h2o_da_len(strings) > 0) {
    i64 idx = h2o_string_findc(strings[0], '=');
    if (idx != -1 && idx < (i64)strings[0]->len) {
      h2o_string_slice name_slice = {.base = strings[0]->base, .len = idx};
      h2o_string_slice value_slice = {.base = strings[0]->base + idx + 1,
                                      .len = strings[0]->len - idx - 1};

      h2o_string *name = h2o_string_dup(pool, &name_slice);
      h2o_string *value = h2o_string_dup(pool, &value_slice);

      h2o_sm_insert(cookie->map, name, value);
      first_attr_idx = 1;
    } else {
      fprintf(stderr, "Malformed cookie: no valid name=value pair found.\n");
      return null;
    }
  }

  if (cookie->map->count == 0) {
    fprintf(stderr, "Malformed cookie: no valid name=value pairs found.\n");
    return null;
  }

  for (u64 i = first_attr_idx; i < h2o_da_len(strings); i++) {
    i64 idx = h2o_string_findc(strings[i], '=');

    if (idx == -1) {
      to_lower(strings[i]);
      if (strings[i]->len == 6 && memcmp(strings[i]->base, "secure", 6) == 0)
        cookie->secure = true;
      else if (strings[i]->len == 8 &&
               memcmp(strings[i]->base, "httponly", 8) == 0)
        cookie->http_only = true;
      continue;
    }

    if (idx >= (i64)strings[i]->len) {
      fprintf(stderr, "Malformed cookie attribute at position %lu\n", i);
      return null;
    }

    for (u64 k = 0; k < (u64)idx; k++) {
      strings[i]->base[k] = tolower(strings[i]->base[k]);
    }

    h2o_string_slice key_slice = {
        .base = strings[i]->base,
        .len = idx,
    };
    h2o_string_slice value_slice = {
        .base = strings[i]->base + idx + 1,
        .len = strings[i]->len - idx - 1,
    };

    if (key_slice.len == 4 && memcmp(key_slice.base, "path", 4) == 0)
      cookie->path = h2o_string_dup(pool, &value_slice);

    else if (key_slice.len == 6 && memcmp(key_slice.base, "domain", 6) == 0)
      cookie->domain = h2o_string_dup(pool, &value_slice);

    else if (key_slice.len == 7 && memcmp(key_slice.base, "max-age", 7) == 0) {
      cstr *endptr = null;
      i64 max_age = strtol(h2o_string_to_cstr(&value_slice), &endptr, 10);
      if (endptr == h2o_string_to_cstr(&value_slice)) {
        fprintf(stderr, "Failed to parse max-age cookie\n");
        return null;
      }

      cookie->max_age = max_age;
    }

    else if (key_slice.len == 8 && memcmp(key_slice.base, "samesite", 8) == 0) {
      h2o_string samesite_value = {value_slice.base, value_slice.len};
      to_lower(&samesite_value);

      if (samesite_value.len == 3 && memcmp(samesite_value.base, "lax", 3) == 0)
        cookie->same_site = LAX;
      else if (samesite_value.len == 6 &&
               memcmp(samesite_value.base, "strict", 6) == 0)
        cookie->same_site = STRICT;
      else if (samesite_value.len == 4 &&
               memcmp(samesite_value.base, "none", 4) == 0)
        cookie->same_site = NONE;
    }

    else if (key_slice.len == 7 && memcmp(key_slice.base, "expires", 7) == 0) {
      i64 expires;
      cstr *expires_cstr = h2o_string_to_cstr(&value_slice);
      if (!validate_expires(expires_cstr, &expires)) {
        fprintf(stderr, "Invalid expires cookie value.\n");
        return null;
      }

      cookie->expires = expires;
      cookie->expires_str = h2o_string_dup(pool, &value_slice);
    }
  }

  return cookie;
}

//
//
//

void h2o_cookie_add_param(h2o_mem_pool_t *pool, h2o_cookie_t *cookie,
                          h2o_cookie_param_t param, ...) {
  va_list args;
  va_start(args, null);

  switch (param) {
  case HTTP_ONLY:
    cookie->http_only = !cookie->http_only;
    break;

  case SECURE:
    cookie->secure = !cookie->secure;
    break;

  case SAME_SITE: {
    h2o_same_site_args_t val = va_arg(args, h2o_same_site_args_t);
    if (val == -1 || val > STRICT) {
      fprintf(stderr, "SameSite is INVALID.\n");
      return;
    }

    cookie->same_site = val;
  } break;

  case PATH: {
    cstr *val = va_arg(args, cstr *);
    if (!val) {
      fprintf(stderr, "Path value is null.\n");
      return;
    }

    if (val[0] != '/') {
      fprintf(stderr, "Path value must start with '/'.\n");
      return;
    }

    cookie->path = h2o_string_from_cstr(pool, val);
  } break;

  case EXPIRES: {
    cstr *val = va_arg(args, cstr *);
    if (!val) {
      fprintf(stderr, "Expires value is null.\n");
      return;
    }

    if (strlen(val) < 29) {
      fprintf(stderr, "Expires value is invalid.\n");
      return;
    }

    i64 expires;
    if (!validate_expires(val, &expires)) {
      fprintf(stderr, "Invalid expires cookie value.\n");
      return;
    }

    cookie->expires = expires;
    cookie->expires_str = h2o_string_from_cstr(pool, val);
  } break;

  case MAX_AGE: {
    i64 val = va_arg(args, i64);
    if (val <= 0) {
      fprintf(stderr, "Max-Age is invalid.\n");
      return;
    }

    cookie->max_age = time(null) + val;
  } break;

  case DOMAIN: {
    cstr *val = va_arg(args, cstr *);
    if (!val) {
      fprintf(stderr, "Domain value is null.\n");
      return;
    }

    cookie->domain = h2o_string_from_cstr(pool, val);
  } break;
  }
}

void h2o_cookie_append(h2o_cookie_t *cookie, const h2o_string *key,
                       const h2o_string *value) {
  htils_assert(cookie && "Cookie is null.");
  htils_assert(key && "Key is null.");
  htils_assert(key->len > 0 && "Key size is zero.");
  htils_assert(key->base && "Key base is null.");

  htils_assert(value && "Value is null.");
  htils_assert(value->len > 0 && "Value size is zero.");
  htils_assert(value->base && "Value base is null.");
  htils_assert(cookie->map && "Cookie's stringmap is null.");

  h2o_stringmap_result_t res = h2o_sm_insert(cookie->map, key, value);
  if (res == CREATED || res == UPDATED)
    return;

  fprintf(stderr, "Failed to append key and value to cookie.\n");
  return;
}

//
//
//

h2o_string *h2o_cookie_to_string(h2o_mem_pool_t *pool, h2o_cookie_t *cookie) {
  if (!cookie) {
    fprintf(stderr, "Cookie is null.\n");
    return null;
  }

  if (!cookie->map && cookie->map->count == 0) {
    fprintf(stderr, "Cookie's stringmap is null.\n");
    return null;
  }

  cstr buf[4096] = {0}; // Max length a cookie can be.
  u64 len = 0;

#define APPEND(str, str_len)                                                   \
  do {                                                                         \
    if (len + (str_len) >= sizeof(buf)) {                                      \
      fprintf(stderr, "Cookie string exceeds buffer size.\n");                 \
      return null;                                                             \
    }                                                                          \
    memcpy(buf + len, str, str_len);                                           \
    len += (str_len);                                                          \
  } while (0)

  b32 first = true;

  for (u64 i = 0; i < cookie->map->capacity; i++) {
    h2o_stringmap_entry_t *entry = &cookie->map->entries[i];

    if (entry->state == OCCUPIED && entry->key && entry->value) {
      if (!first) {
        APPEND("; ", 2);
      }
      first = false;
      h2o_string *value = entry->value;
      h2o_string *key = entry->key;

      APPEND(key->base, key->len);
      APPEND("=", 1);

      APPEND(value->base, value->len);
    }
  }

  b32 has_more_attr = false;

  if (cookie->domain && cookie->domain->base && cookie->domain->len > 0) {
    APPEND("Domain=", 7);
    APPEND(cookie->domain->base, cookie->domain->len);
    has_more_attr = true;
  }

  if (cookie->path && cookie->path->base && cookie->path->len > 0) {
    if (has_more_attr)
      APPEND("; ", 2);

    APPEND("Path=", 5);
    APPEND(cookie->path->base, cookie->path->len);
    has_more_attr = true;
  }

  if (cookie->expires_str && cookie->expires_str->base &&
      cookie->expires_str->len > 0) {
    if (has_more_attr)
      APPEND("; ", 2);

    APPEND("Expires=", 8);
    APPEND(cookie->expires_str->base, cookie->expires_str->len);
    has_more_attr = true;
  }

  if (cookie->max_age > 0) {
    if (has_more_attr)
      APPEND("; ", 2);

    cstr max_age_buf[32];

    APPEND("Max-Age=", 8);
    i64 written = snprintf(max_age_buf, 32, "%ld", cookie->max_age);
    APPEND(max_age_buf, written);
    has_more_attr = true;
  }

  if (cookie->same_site != INVALID) {
    if (has_more_attr)
      APPEND("; ", 2);

    APPEND("SameSite=", 9);

    switch (cookie->same_site) {
    case NONE:
      APPEND("None", 4);
      break;
    case LAX:
      APPEND("Lax", 3);
      break;
    case STRICT:
      APPEND("Strict", 6);
      break;
    default:
      fprintf(stderr, "Unknown SameSite value.\n");
      return null;
    }

    has_more_attr = true;
  }

  if (cookie->secure) {
    if (has_more_attr)
      APPEND("; ", 2);

    APPEND("Secure", 6);
    has_more_attr = true;
  }

  if (cookie->http_only) {
    if (has_more_attr)
      APPEND("; ", 2);

    APPEND("HttpOnly", 8);
    has_more_attr = true;
  }

#undef APPEND

  h2o_string *str = h2o_string_new(pool, len);
  memcpy(str->base, buf, len);

  return str;
}

h2o_string *h2o_cookie_get_value(h2o_cookie_t *cookie, const h2o_string *key) {
  htils_assert(cookie && "Cookie is null.");
  htils_assert(key && "Key is null.");
  htils_assert(key->len > 0 && "Key size is zero.");
  htils_assert(key->base && "Key base is null.");
  htils_assert(cookie->map && "Cookie's stringmap is null.");

  if (cookie->map->count == 0) {
    fprintf(stderr, "Cookie has no values.\n");
    return null;
  }

  return (h2o_string *)h2o_sm_get(cookie->map, key);
}
