#include <ctype.h>
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

  if (strlen(date_str) < 28) {
    fprintf(stderr, "IMF-fixdate string is too short.\n");
    return false;
  }

  if (date_str[3] != ',' && date_str[4] != ' ') {
    fprintf(stderr, "IMF-fixdate string is invalid.\n");
    return false;
  }

  day = parse_str_to_num(&date_str[5], 2);
  month = parse_3let_month(&date_str[7]);
  year = parse_str_to_num(&date_str[11], 4);
  hour = parse_str_to_num(&date_str[16], 2);
  min = parse_str_to_num(&date_str[19], 2);
  sec = parse_str_to_num(&date_str[22], 2);

  if (date_str[25] != 'g' && date_str[26] != 'm' && date_str[27] != 't') {
    fprintf(stderr, "IMF-fixdate string is invalid, missing 'GMT'.\n");
    fprintf(stderr, "Invalid IMF-fixdate string: %s\n", date_str);
    fprintf(stderr, "Expected 'G' at position 25 found %c\n", date_str[25]);
    fprintf(stderr, "Expected 'M' at position 26 found %c\n", date_str[26]);
    fprintf(stderr, "Expected 'T' at position 27 found %c\n", date_str[27]);
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

static b32 parse_rfc_850(const cstr *date_str, time_t *result) {
  struct tm tm = {0};
  i32 month = 0, day = 0, year = 0, hour = 0, min = 0, sec = 0;

  if (date_str[8] != ',' && date_str[9] != ' ')
    return false;

  if (date_str[12] != '-')
    return false;

  day = parse_str_to_num(&date_str[10], 2);
  month = parse_3let_month(&date_str[13]);
  year = parse_str_to_num(&date_str[17], 2);
  hour = parse_str_to_num(&date_str[20], 2);
  min = parse_str_to_num(&date_str[23], 2);
  sec = parse_str_to_num(&date_str[26], 2);

  if (date_str[29] != 'G' && date_str[30] != 'M' && date_str[31] != 'T')
    return false;

  if (year < 50)
    year += 2000;
  else
    year += 1900;

  if (month >= 0 && day >= 1 && day <= 31 && hour >= 0 && hour < 24 &&
      min >= 0 && min < 60 && sec >= 0 && sec < 60) {
    tm.tm_mday = day;
    printf("tm.tm_mday: %d\n", tm.tm_mday);
    tm.tm_mon = month;
    printf("tm.tm_mon: %d\n", tm.tm_mon);
    tm.tm_year = year - 1900;
    printf("tm.tm_year: %d\n", tm.tm_year);
    tm.tm_hour = hour;
    printf("tm.tm_hour: %d\n", tm.tm_hour);
    tm.tm_min = min;
    printf("tm.tm_min: %d\n", tm.tm_min);
    tm.tm_sec = sec;
    printf("tm.tm_sec: %d\n", tm.tm_sec);
    tm.tm_isdst = 0;

    *result = timegm(&tm);
    return (*result != -1);
  }

  return false;
}

static b32 parse_asctime(const cstr *date_str, time_t *result) {
  struct tm tm = {0};
  i32 month = 0, day = 0, year = 0, hour = 0, min = 0, sec = 0;

  if (date_str[3] != ' ')
    return false;

  month = parse_3let_month(&date_str[4]);
  day = parse_str_to_num(&date_str[8], 2);
  hour = parse_str_to_num(&date_str[11], 2);
  min = parse_str_to_num(&date_str[14], 2);
  sec = parse_str_to_num(&date_str[17], 2);
  year = parse_str_to_num(&date_str[20], 4);

  if (month >= 0 && day >= 1 && day <= 31 && year >= 1970 && hour >= 0 &&
      hour < 24 && min >= 0 && min < 60 && sec >= 0 && sec < 60) {
    tm.tm_mday = day;
    printf("tm.tm_mday: %d\n", tm.tm_mday);
    tm.tm_mon = month;
    printf("tm.tm_mon: %d\n", tm.tm_mon);
    tm.tm_year = year - 1900;
    printf("tm.tm_year: %d\n", tm.tm_year);
    tm.tm_hour = hour;
    printf("tm.tm_hour: %d\n", tm.tm_hour);
    tm.tm_min = min;
    printf("tm.tm_min: %d\n", tm.tm_min);
    tm.tm_sec = sec;
    printf("tm.tm_sec: %d\n", tm.tm_sec);
    tm.tm_isdst = 0;

    *result = timegm(&tm);
    return (*result != -1);
  }

  return false;
}

static b32 parse_http_date(const cstr *date_cstr, time_t *result) {
  if (date_cstr[3] == ',' && date_cstr[4] == ' ') {
    fprintf(stderr, "Trying IMF-fixdate.\n");
    return parse_imf_fixdate(date_cstr, result);
  } else if (date_cstr[8] == ',' && date_cstr[9] == ' ') {
    fprintf(stderr, "Trying RFC-850.\n");
    return parse_rfc_850(date_cstr, result);
  } else if (date_cstr[3] == ' ') {
    fprintf(stderr, "Trying asctime.\n");
    return parse_asctime(date_cstr, result);
  }

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

h2o_cookie_t *h2o_cookie_new(h2o_mem_pool_t *pool, const h2o_string *name,
                             const h2o_string *value) {
  h2o_cookie_t *cookie = h2o_mem_alloc_pool(pool, h2o_cookie_t, 1);
  *cookie = (h2o_cookie_t){0};
  cookie->name = h2o_string_dup(pool, name);
  cookie->value = h2o_string_dup(pool, value);
  cookie->max_age = -1;
  cookie->expires = -1;
  cookie->same_site = INVALID;

  return cookie;
}

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

  i64 idx = h2o_string_findc(strings[0], '=');
  if (idx == -1) {
    fprintf(stderr, "Cookie string is invalid.\n");
    return null;
  }

  h2o_cookie_t *cookie = h2o_mem_alloc_pool(pool, h2o_cookie_t, 1);
  *cookie = (h2o_cookie_t){0};
  cookie->same_site = INVALID;
  cookie->max_age = -1;

  h2o_string_slice cookie_name_slice = (h2o_string_slice){
      .base = strings[0]->base,
      .len = idx,
  };
  h2o_string_slice cookie_value_slice = (h2o_string_slice){
      .base = strings[0]->base + idx + 1,
      .len = strings[0]->len - idx - 1,
  };

  cookie->name = h2o_string_dup(pool, &cookie_name_slice);
  cookie->value = h2o_string_dup(pool, &cookie_value_slice);

  for (u64 i = 1; i < h2o_da_len(strings); i++) {
    to_lower(strings[i]);

    idx = h2o_string_findc(strings[i], '=');
    h2o_string_slice key_slice = {0};
    h2o_string_slice value_slice = {0};

    if (idx == -1) {
      if (strings[i]->len == 6 && memcmp(strings[i]->base, "secure", 6) == 0)
        cookie->secure = true;
      else if (strings[i]->len == 8 &&
               memcmp(strings[i]->base, "httponly", 8) == 0)
        cookie->http_only = true;
      continue;
    }

    if (idx >= (i64)strings[i]->len) {
      fprintf(stderr, "Malformed cookie attribute\n");
      return null;
    }

    key_slice = (h2o_string_slice){
        .base = strings[i]->base,
        .len = idx,
    };

    value_slice = (h2o_string_slice){
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
      if (value_slice.len == 3 && memcmp(value_slice.base, "lax", 3) == 0)
        cookie->same_site = LAX;
      else if (value_slice.len == 6 &&
               memcmp(value_slice.base, "strict", 6) == 0)
        cookie->same_site = STRICT;
      else if (value_slice.len == 4 && memcmp(value_slice.base, "none", 4) == 0)
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
      cookie->expires_str = h2o_string_from_cstr(pool, expires_cstr);
    }
  }

  return cookie;
}
