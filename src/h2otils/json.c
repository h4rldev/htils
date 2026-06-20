#include <ctype.h>
#include <math.h>
#include <threads.h>

//
//
//

#include <h2o/memory.h>

//
//
//

#include <htils/assert.h>
#include <htils/basictypes.h>

//
//
//

#include <h2otils/darray.h>
#include <h2otils/json.h>
#include <h2otils/string.h>
#include <h2otils/stringmap.h>

//
//
//

static inline b32 is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void skip_whitespace(h2o_string_slice slice, u64 *pos) {
  while (*pos < slice.len && is_whitespace(slice.base[*pos]))
    (*pos)++;
}

static h2o_json_type_t determine_value_type(h2o_string_slice slice, u64 *pos,
                                            u64 *end_pos) {
  skip_whitespace(slice, pos);

  if (*pos >= slice.len)
    return H2O_JSON_INVALID;

  u64 local_pos = *pos;
  char c = slice.base[local_pos];

  switch (c) {
  case '{':
    local_pos++;
    skip_whitespace(slice, &local_pos);
    if (local_pos < slice.len && slice.base[local_pos] == '}') {
      *end_pos = local_pos + 1;
      return H2O_JSON_OBJECT;
    }
    *end_pos = local_pos;
    return H2O_JSON_OBJECT;
  case '[':
    local_pos++;
    skip_whitespace(slice, &local_pos);
    if (local_pos < slice.len && slice.base[local_pos] == ']') {
      *end_pos = local_pos + 1;
      return H2O_JSON_ARRAY;
    }
    *end_pos = local_pos;
    return H2O_JSON_ARRAY;

  case '"':
    local_pos++;
    while (local_pos < slice.len && slice.base[local_pos] != '"') {
      if (slice.base[local_pos] == '\\' && local_pos + 1 < slice.len)
        local_pos++;
      local_pos++;
    }

    if (local_pos < slice.len)
      local_pos++;

    *end_pos = local_pos;
    return H2O_JSON_STRING;

  case 't':
    if (*pos + 3 < slice.len && memcmp(slice.base + *pos, "true", 4) == 0) {
      *end_pos = local_pos + 4;
      return H2O_JSON_BOOL;
    }
    return H2O_JSON_INVALID;

  case 'f':
    if (*pos + 4 < slice.len && memcmp(slice.base + *pos, "false", 5) == 0) {
      *end_pos = local_pos + 5;
      return H2O_JSON_BOOL;
    }
    return H2O_JSON_INVALID;

  case 'n':
    if (*pos + 3 < slice.len && memcmp(slice.base + *pos, "null", 4) == 0) {
      *end_pos = local_pos + 4;
      return H2O_JSON_NULL;
    }
    return H2O_JSON_INVALID;

  case '-':
  default:
    if (c == '-' || isdigit(c)) {
      b32 is_float = false;

      if (c == '-')
        local_pos++;

      while (local_pos < slice.len && isdigit(slice.base[local_pos]))
        local_pos++;

      if (local_pos < slice.len && slice.base[local_pos] == '.') {
        local_pos++;
        is_float = true;

        while (local_pos < slice.len && isdigit(slice.base[local_pos]))
          local_pos++;
      }

      if (local_pos < slice.len &&
          (slice.base[local_pos] == 'e' || slice.base[local_pos] == 'E')) {
        local_pos++;
        is_float = true;

        if (local_pos < slice.len &&
            (slice.base[local_pos] == '+' || slice.base[local_pos] == '-'))
          local_pos++;

        while (local_pos < slice.len && isdigit(slice.base[local_pos]))
          local_pos++;
      }

      *end_pos = local_pos;
      return is_float ? H2O_JSON_FLOAT : H2O_JSON_INTEGER;
    }
    return H2O_JSON_INVALID;
  }
}

static void skip_json_value(h2o_string_slice slice, u64 *pos) {
  if (*pos >= slice.len)
    return;

  skip_whitespace(slice, pos);

  if (*pos >= slice.len)
    return;

  u64 end_pos = *pos;
  h2o_json_type_t type = determine_value_type(slice, pos, &end_pos);

  if (type != H2O_JSON_INVALID)
    *pos = end_pos;
}

//
//
//

static thread_local h2o_json_error_t global_json_error = {0};

static void set_error(h2o_mem_pool_t *pool, cstr *msg, u64 pos) {
  u64 len = strlen(msg);
  cstr *buf = h2o_mem_alloc_pool(pool, char, len + 1);
  memcpy(buf, msg, len);
  buf[len] = '\0';

  global_json_error.msg = buf;
  global_json_error.has_error = true;
  global_json_error.pos = pos;
}

static b32 parse_number(h2o_mem_pool_t *pool, h2o_string_slice slice,
                        u64 num_start_pos, u64 num_len, b32 is_float,
                        h2o_json_node_t *node, u64 pos) {
  char buf[21];
  char *num_str;
  b32 use_pool = num_len >= 21;

  if (use_pool) {
    num_str = h2o_mem_alloc_pool(pool, char, num_len + 1);
  } else {
    num_str = buf;
  }

  memcpy(num_str, slice.base + num_start_pos, num_len);
  num_str[num_len] = '\0';

  b32 parse_as_float = is_float || num_len >= 21;

  if (parse_as_float) {
    char *endptr;
    f64 float_val = strtod(num_str, &endptr);

    if (endptr == num_str) {
      set_error(pool, "Failed to parse JSON float.", pos);
      node->type = H2O_JSON_INVALID;
      return false;
    }

    node->type = H2O_JSON_FLOAT;
    node->data.float_data.float_val = float_val;
  } else {
    char *endptr;
    i64 int_val = strtoll(num_str, &endptr, 10);

    if (endptr == num_str) {
      set_error(pool, "Failed to parse JSON integer.", pos);
      node->type = H2O_JSON_INVALID;
      return false;
    }

    node->type = H2O_JSON_INTEGER;
    node->data.int_data.int_val = int_val;
  }

  return true;
}

//
//
//

/// Full
static h2o_json_node_t *parse_object_full(h2o_mem_pool_t *pool,
                                          h2o_string_slice slice, u64 *pos);
static h2o_json_node_t *parse_array_full(h2o_mem_pool_t *pool,
                                         h2o_string_slice slice, u64 *pos);

static h2o_json_node_t *parse_primitive_full(h2o_mem_pool_t *pool,
                                             h2o_string_slice slice, u64 *pos) {
  skip_whitespace(slice, pos);

  if (*pos >= slice.len) {
    set_error(pool, "Unexpected end of JSON string.", *pos);
    return null;
  }

  u64 start_pos = *pos;
  char c = slice.base[*pos];

  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);

  switch (c) {
  case 't':
    if (*pos + 3 < slice.len && memcmp(slice.base + *pos, "true", 4) == 0) {
      *pos += 4;
      node->type = H2O_JSON_BOOL;
      node->data.bool_data.bool_val = true;
      node->slice =
          (h2o_string_slice){.base = slice.base + start_pos, .len = 4};
      return node;
    }
    set_error(pool, "Invalid JSON boolean value.", *pos);
    return null;
  case 'f':
    if (*pos + 4 < slice.len && memcmp(slice.base + *pos, "false", 5) == 0) {
      *pos += 5;
      node->type = H2O_JSON_BOOL;
      node->data.bool_data.bool_val = false;
      node->slice =
          (h2o_string_slice){.base = slice.base + start_pos, .len = 5};
      return node;
    }
    set_error(pool, "Invalid JSON boolean value.", *pos);
    return null;

  case 'n':
    if (*pos + 3 < slice.len && memcmp(slice.base + *pos, "null", 4) == 0) {
      *pos += 4;
      node->type = H2O_JSON_NULL;
      node->slice =
          (h2o_string_slice){.base = slice.base + start_pos, .len = 4};
      return node;
    }
    set_error(pool, "Invalid JSON null value.", *pos);
    return null;

  case '-':
  default:
    if (c == '-' || isdigit(c)) {
      u64 num_start_pos = *pos;
      b32 is_float = false;

      if (c == '-') {
        (*pos)++;
        if (*pos >= slice.len || !isdigit(slice.base[*pos])) {
          set_error(pool, "Invalid number format", start_pos);
          return null;
        }
      }

      while (*pos < slice.len && isdigit(slice.base[*pos]))
        (*pos)++;

      if (*pos < slice.len && slice.base[*pos] == '.') {
        (*pos)++;
        is_float = true;

        if (*pos >= slice.len || !isdigit(slice.base[*pos])) {
          set_error(pool, "Invalid number format after decimal point",
                    start_pos);
          return null;
        }

        while (*pos < slice.len && isdigit(slice.base[*pos]))
          (*pos)++;
      }

      if (*pos < slice.len &&
          (slice.base[*pos] == 'e' || slice.base[*pos] == 'E')) {
        (*pos)++;
        is_float = true;

        if (*pos < slice.len &&
            (slice.base[*pos] == '+' || slice.base[*pos] == '-'))
          (*pos)++;

        if (*pos >= slice.len || !isdigit(slice.base[*pos])) {
          set_error(pool,
                    "Invalid number format, expected digits after exponent",
                    start_pos);
          return null;
        }

        if (*pos < slice.len && isdigit(slice.base[*pos]))
          (*pos)++;
      }

      u64 num_len = *pos - num_start_pos;
      if (!parse_number(pool, slice, num_start_pos, num_len, is_float, node,
                        *pos))
        return node;

      node->slice = (h2o_string_slice){.base = slice.base + num_start_pos,
                                       .len = num_len};
      return node;
    }

    set_error(pool, "Invalid JSON value.", *pos);
    return null;
  }
}

static h2o_json_node_t *parse_value_full(h2o_mem_pool_t *pool,
                                         h2o_string_slice slice, u64 *pos) {
  skip_whitespace(slice, pos);

  if (*pos >= slice.len) {
    set_error(pool, "Unexpected end of JSON string.", *pos);
    return null;
  }

  char c = slice.base[*pos];
  switch (c) {
  case '{':
    return parse_object_full(pool, slice, pos);
  case '[':
    return parse_array_full(pool, slice, pos);
  case '"':
    (*pos)++;
    u64 str_start_pos = *pos;
    while (*pos < slice.len && slice.base[*pos] != '"') {
      if (slice.base[*pos] == '\\' && *pos + 1 < slice.len)
        (*pos)++;
      (*pos)++;
    }

    h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
    node->type = H2O_JSON_STRING;
    node->slice = (h2o_string_slice){
        .base = slice.base + str_start_pos,
        .len = *pos - str_start_pos + 1,
    };
    node->data.str_data.str_val = (h2o_string_slice){
        .base = slice.base + str_start_pos,
        .len = *pos - str_start_pos,
    };
    (*pos)++;
    return node;

  default:
    return parse_primitive_full(pool, slice, pos);
  }
}

static h2o_json_node_t *parse_object_full(h2o_mem_pool_t *pool,
                                          h2o_string_slice slice, u64 *pos) {
  u64 start_pos = *pos;

  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  node->type = H2O_JSON_OBJECT;
  node->slice = (h2o_string_slice){.base = slice.base + start_pos, .len = 0};
  node->data.object_data.is_lazy = false;

  node->data.object_data.full.nodes = h2o_sm_new(pool, 16);
  node->data.object_data.keys = h2o_sm_new(pool, 16);
  node->data.object_data.lazy.lazy_poses = null;

  (*pos)++;
  skip_whitespace(slice, pos);

  if (*pos < slice.len && slice.base[*pos] == '}') {
    (*pos)++;
    node->slice.len = slice.base + *pos - node->slice.base;
    return node;
  }

  while (*pos < slice.len) {
    skip_whitespace(slice, pos);

    if (*pos >= slice.len || slice.base[*pos] == '}') {
      (*pos)++;
      break;
    }

    if (*pos >= slice.len || slice.base[*pos] != '"') {
      set_error(pool, "Expected '\"' for object key.", *pos);
      node->type = H2O_JSON_INVALID;
      return node;
    }

    u64 key_start = ++(*pos);
    while (*pos < slice.len && slice.base[*pos] != '"') {
      if (slice.base[*pos] == '\\' && *pos + 1 < slice.len)
        (*pos)++;
      (*pos)++;
    }

    h2o_string_slice key_slice = (h2o_string_slice){
        .base = slice.base + key_start,
        .len = *pos - key_start,
    };

    (*pos)++;
    skip_whitespace(slice, pos);

    if (*pos >= slice.len || slice.base[*pos] != ':') {
      set_error(pool, "Expected ':' after object key.", *pos);
      node->type = H2O_JSON_INVALID;
      return node;
    }

    (*pos)++;
    skip_whitespace(slice, pos);

    h2o_string *key_string = h2o_string_dup(pool, &key_slice);
    h2o_json_node_t *value = parse_value_full(pool, slice, pos);

    if (!value || value->type == H2O_JSON_INVALID) {
      node->type = H2O_JSON_INVALID;
      return node;
    }

    h2o_sm_insert(node->data.object_data.keys, key_string, key_string);
    h2o_sm_insert(node->data.object_data.full.nodes, key_string, value);

    skip_whitespace(slice, pos);

    if (*pos >= slice.len)
      break;

    if (slice.base[*pos] == '}') {
      (*pos)++;
      break;
    }

    if (slice.base[*pos] == ',') {
      (*pos)++;
      skip_whitespace(slice, pos);

      if (*pos >= slice.len || slice.base[*pos] == '}') {
        set_error(pool, "Trailing comma in object.", *pos);
        node->type = H2O_JSON_INVALID;
        return node;
      }

      continue;
    }
  }

  node->slice.len = slice.base + *pos - node->slice.base;
  return node;
}

static h2o_json_node_t *parse_array_full(h2o_mem_pool_t *pool,
                                         h2o_string_slice slice, u64 *pos) {
  u64 start_pos = *pos;

  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  node->type = H2O_JSON_ARRAY;
  node->slice = (h2o_string_slice){.base = slice.base + start_pos, .len = 0};
  node->data.array_data.is_lazy = false;

  h2o_da_new(pool, node->data.array_data.full.items, 16);
  node->data.array_data.lazy.poses = null;

  (*pos)++;
  skip_whitespace(slice, pos);

  if (*pos < slice.len && slice.base[*pos] == ']') {
    (*pos)++;
    node->slice.len = slice.base + *pos - node->slice.base;
    return node;
  }

  while (*pos < slice.len) {
    skip_whitespace(slice, pos);

    if (*pos >= slice.len || slice.base[*pos] == ']') {
      (*pos)++;
      break;
    }

    h2o_json_node_t *element = parse_value_full(pool, slice, pos);

    if (!element || element->type == H2O_JSON_INVALID) {
      node->type = H2O_JSON_INVALID;
      return node;
    }

    h2o_da_append(pool, node->data.array_data.full.items, element);
    skip_whitespace(slice, pos);

    if (*pos >= slice.len)
      break;

    if (slice.base[*pos] == ']') {
      (*pos)++;
      break;
    }

    if (slice.base[*pos] == ',') {
      (*pos)++;
      skip_whitespace(slice, pos);

      if (*pos >= slice.len || slice.base[*pos] == ']') {
        set_error(pool, "Trailing comma in array.", *pos);
        node->type = H2O_JSON_INVALID;
        return node;
      }

      continue;
    }
  }

  node->slice.len = slice.base + *pos - node->slice.base;
  return node;
}

static h2o_json_node_t *parse_array_lazy(h2o_mem_pool_t *pool,
                                         h2o_string_slice slice, u64 *pos);
static h2o_json_node_t *parse_object_lazy(h2o_mem_pool_t *pool,
                                          h2o_string_slice slice, u64 *pos);

static h2o_json_node_t *parse_value_lazy(h2o_mem_pool_t *pool,
                                         h2o_string_slice slice, u64 *pos) {
  skip_whitespace(slice, pos);

  if (*pos >= slice.len) {
    set_error(pool, "Unexpected end of JSON string.", *pos);
    return null;
  }

  u64 start_pos = *pos;
  char c = slice.base[*pos];

  h2o_json_node_t *node;

  switch (c) {
  case '{':
    node = parse_object_lazy(pool, slice, pos);
    break;
  case '[':
    node = parse_array_lazy(pool, slice, pos);
    break;
  case '"':
    (*pos)++;
    u64 str_start_pos = *pos;
    while (*pos < slice.len && slice.base[*pos] != '"') {
      if (slice.base[*pos] == '\\' && *pos + 1 < slice.len)
        (*pos)++;
      (*pos)++;
    }

    if (*pos >= slice.len) {
      set_error(pool, "Unexpected end of JSON string.", start_pos);
      return null;
    }

    node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
    node->type = H2O_JSON_STRING;
    node->slice = (h2o_string_slice){
        .base = slice.base + start_pos,
        .len = *pos - start_pos + 1,
    };

    node->data.str_data.str_val = (h2o_string_slice){
        .base = slice.base + str_start_pos,
        .len = *pos - str_start_pos,
    };

    (*pos)++;
    break;

  case 't':
  case 'f':
  case 'n':
  case '-':
  default:
    node = parse_primitive_full(pool, slice, pos);
    break;
  }

  return node;
}

//
//
//

// Lazy

static h2o_json_node_t *parse_object_lazy(h2o_mem_pool_t *pool,
                                          h2o_string_slice slice, u64 *pos) {
  u64 start_pos = *pos;

  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  node->type = H2O_JSON_OBJECT;
  node->slice = (h2o_string_slice){.base = slice.base + start_pos, .len = 0};
  node->data.object_data.is_lazy = true;

  node->data.object_data.keys = h2o_sm_new(pool, 16);
  node->data.object_data.lazy.lazy_poses = h2o_sm_new(pool, 16);
  node->data.object_data.full.nodes = null;

  (*pos)++;
  skip_whitespace(slice, pos);

  if (*pos < slice.len && slice.base[*pos] == '}') {
    (*pos)++;
    node->slice.len = slice.base + *pos - node->slice.base;
    return node;
  }

  while (*pos < slice.len) {
    skip_whitespace(slice, pos);

    if (*pos >= slice.len || slice.base[*pos] == '}') {
      (*pos)++;
      break;
    }

    if (*pos >= slice.len || slice.base[*pos] != '"') {
      set_error(pool, "Expected '\"' for object key.", *pos);
      node->type = H2O_JSON_INVALID;
      return node;
    }

    u64 key_start = ++(*pos);
    while (*pos < slice.len && slice.base[*pos] != '"') {
      if (slice.base[*pos] == '\\' && *pos + 1 < slice.len)
        (*pos)++;
      (*pos)++;
    }

    h2o_string_slice key_slice = (h2o_string_slice){
        .base = slice.base + key_start,
        .len = *pos - key_start,
    };

    (*pos)++;
    skip_whitespace(slice, pos);

    if (*pos >= slice.len || slice.base[*pos] != ':') {
      set_error(pool, "Expected ':' after object key.", *pos);
      node->type = H2O_JSON_INVALID;
      return node;
    }

    (*pos)++;
    skip_whitespace(slice, pos);

    u64 value_pos = *pos;
    u64 value_end_pos = *pos;
    h2o_json_type_t value_type =
        determine_value_type(slice, &value_pos, &value_end_pos);
    h2o_string *key_string = h2o_string_dup(pool, &key_slice);

    h2o_sm_insert(node->data.object_data.keys, key_string, key_string);

    struct {
      u64 pos;
      h2o_json_type_t type;
    } lazy_value_info;

    lazy_value_info.pos = value_pos;
    lazy_value_info.type = value_type;

    h2o_sm_insert(node->data.object_data.lazy.lazy_poses, key_string,
                  &lazy_value_info);

    *pos = value_end_pos;
    skip_whitespace(slice, pos);

    if (*pos >= slice.len)
      break;

    if (slice.base[*pos] == '}') {
      (*pos)++;
      break;
    }

    if (slice.base[*pos] == ',') {
      (*pos)++;
      skip_whitespace(slice, pos);

      if (*pos >= slice.len || slice.base[*pos] == '}') {
        set_error(pool, "Trailing comma in object.", *pos);
        node->type = H2O_JSON_INVALID;
        return node;
      }

      continue;
    }
  }

  node->slice.len = slice.base + *pos - node->slice.base;
  return node;
}

static h2o_json_node_t *parse_array_lazy(h2o_mem_pool_t *pool,
                                         h2o_string_slice slice, u64 *pos) {
  u64 start_pos = *pos;

  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  node->type = H2O_JSON_ARRAY;
  node->slice = (h2o_string_slice){.base = slice.base + start_pos, .len = 0};
  node->data.array_data.is_lazy = true;

  (*pos)++;
  skip_whitespace(slice, pos);

  if (*pos < slice.len && slice.base[*pos] == ']') {
    (*pos)++;
    node->slice.len = slice.base + *pos - node->slice.base;
    return node;
  }

  while (*pos < slice.len) {
    skip_whitespace(slice, pos);

    if (*pos >= slice.len || slice.base[*pos] == ']') {
      (*pos)++;
      break;
    }

    h2o_da_append(pool, node->data.array_data.lazy.poses, *pos);
    node->data.array_data.lazy.count++;

    skip_json_value(slice, pos);
    skip_whitespace(slice, pos);

    if (*pos >= slice.len)
      break;

    if (slice.base[*pos] == ']') {
      (*pos)++;
      break;
    }

    if (slice.base[*pos] == ',') {
      (*pos)++;
      skip_whitespace(slice, pos);

      if (*pos >= slice.len || slice.base[*pos] == ']') {
        set_error(pool, "Trailing comma in array.", *pos);
        node->type = H2O_JSON_INVALID;
        return node;
      }

      continue;
    }
  }

  node->slice.len = slice.base + *pos - node->slice.base;
  return node;
}

//
//
//

static void escape_json_string(h2o_mem_pool_t *pool, h2o_string *out,
                               h2o_string_slice in) {
  for (u64 i = 0; i < in.len; i++) {
    char c = in.base[i];
    switch (c) {
    case '"':
      h2o_string_concatf(pool, out, "\\\"");
      break;

    case '\\':
      h2o_string_concatf(pool, out, "\\\\");
      break;

    case '\b':
      h2o_string_concatf(pool, out, "\\b");
      break;

    case '\f':
      h2o_string_concatf(pool, out, "\\f");
      break;
    case '\n':
      h2o_string_concatf(pool, out, "\\n");
      break;

    case '\r':
      h2o_string_concatf(pool, out, "\\r");
      break;

    case '\t':
      h2o_string_concatf(pool, out, "\\t");
      break;

    default:
      if (c < 0x20)
        h2o_string_concatf(pool, out, "\\u%04x", (ucstr)c);
      else
        h2o_string_concatb(pool, out, &in, 1);
      break;
    }
  }
}

static void write_json_value(h2o_mem_pool_t *pool, h2o_string *out,
                             h2o_json_node_t *node, u64 indent, b32 pretty) {
  if (!node || node->type == H2O_JSON_INVALID) {
    h2o_string_concatf(pool, out, "{}");
    return;
  }

  switch (node->type) {
  case H2O_JSON_NULL:
    h2o_string_concatf(pool, out, "null");
    break;
  case H2O_JSON_BOOL:
    h2o_string_concatf(pool, out, "%s",
                       node->data.bool_data.bool_val ? "true" : "false");
    break;
  case H2O_JSON_INTEGER:
    h2o_string_concatf(pool, out, "%lld",
                       (long long)node->data.int_data.int_val);
    break;
  case H2O_JSON_FLOAT:
    h2o_string_concatf(pool, out, "%g", node->data.float_data.float_val);
    break;
  case H2O_JSON_STRING:
    h2o_string_concatf(pool, out, "\"");
    escape_json_string(pool, out, node->data.str_data.str_val);
    h2o_string_concatf(pool, out, "\"");
    break;
  case H2O_JSON_ARRAY:
    h2o_string_concatf(pool, out, "[");
    u64 current_indent = indent + (pretty ? 2 : 0);
    for (u64 i = 0; i < h2o_da_len(node->data.array_data.full.items); i++) {
      h2o_json_node_t *child = h2o_json_get_array(pool, node, i);
      if (!child)
        continue;

      if (pretty && i > 0) {
        h2o_string_concatf(pool, out, ",\n");
        for (u64 j = 0; j < current_indent; j++)
          h2o_string_concatf(pool, out, " ");
      } else if (i > 0)
        h2o_string_concatf(pool, out, ",");

      write_json_value(pool, out, child, current_indent, pretty);
    }
    h2o_string_concatf(pool, out, "]");
    break;
  case H2O_JSON_OBJECT:
    h2o_string_concatf(pool, out, "{");
    if (!node->data.object_data.full.nodes) {
      h2o_string_concatf(pool, out, "}");
      break;
    }

    u64 obj_indent = indent + (pretty ? 2 : 0);
    b32 first = true;

    for (u64 i = 0; i < node->data.object_data.keys->capacity; i++) {
      h2o_stringmap_entry_t entry = node->data.object_data.keys->entries[i];
      if (entry.state != OCCUPIED)
        continue;

      h2o_string *key = (h2o_string *)entry.value;
      h2o_json_node_t *val =
          (h2o_json_node_t *)h2o_sm_get(node->data.object_data.full.nodes, key);

      if (!val)
        continue;

      if (pretty && !first) {
        h2o_string_concatf(pool, out, ",\n");
        for (u64 j = 0; j < obj_indent; j++)
          h2o_string_concatf(pool, out, " ");
      } else if (!first)
        h2o_string_concatf(pool, out, ",");

      h2o_string_slice key_slice = h2o_string_slice_from_h2o_string(key);
      h2o_string_concatf(pool, out, "\"");
      escape_json_string(pool, out, key_slice);
      h2o_string_concatf(pool, out, "\":");

      if (pretty)
        h2o_string_concatf(pool, out, " ");

      write_json_value(pool, out, val, obj_indent, pretty);
      first = false;
    }

    h2o_string_concatf(pool, out, "}");
    break;

  default:
    h2o_string_concatf(pool, out, "null");
    break;
  }
}

static u64 estimate_json_size(h2o_json_node_t *node) {
  if (!node)
    return 5;

  switch (node->type) {
  case H2O_JSON_NULL:
    return 5;
  case H2O_JSON_BOOL:
    return node->data.bool_data.bool_val ? 5 : 6; // "true" or "false"
  case H2O_JSON_INTEGER:
    return 21;
  case H2O_JSON_FLOAT:
    return 31;
  case H2O_JSON_STRING:
    return node->data.str_data.str_val.len * 2 + 4;
  case H2O_JSON_ARRAY: {
    u64 total = 2;
    for (u64 i = 0; i < h2o_da_len(node->data.array_data.full.items); i++) {
      total += estimate_json_size(node->data.array_data.full.items[i]);
      if (i > 0)
        total++;
    }
    return total + 100;
  }

  case H2O_JSON_OBJECT: {
    u64 total = 2;
    if (node->data.object_data.full.nodes) {
      for (u64 i = 0; i < node->data.object_data.full.nodes->capacity; i++) {
        h2o_stringmap_entry_t entry =
            node->data.object_data.full.nodes->entries[i];
        if (entry.state == OCCUPIED) {
          h2o_string *key = (h2o_string *)entry.key;
          total += key->len * 2 + 4;
          total += 1;
          h2o_json_node_t *value = (h2o_json_node_t *)entry.value;
          total += estimate_json_size(value);
          total += 1;
        }
      }
    }
    return total + 200;
  }

  default:
    return 5;
  }
}

//
//
//

h2o_json_error_t *h2o_json_get_error(h2o_mem_pool_t *pool) {
  if (!global_json_error.has_error)
    return null;

  h2o_json_error_t *err = h2o_mem_alloc_pool(pool, h2o_json_error_t, 1);
  *err = global_json_error;
  return err;
}

void h2o_json_clear_error(void) { global_json_error = (h2o_json_error_t){0}; }

//
//
//

h2o_json_node_t *h2o_json_parse(h2o_mem_pool_t *pool,
                                const h2o_string *json_body) {
  h2o_string_slice slice = h2o_string_slice_from_h2o_string(json_body);
  return h2o_json_parse_from_slice(pool, slice);
}

h2o_json_node_t *h2o_json_parse_from_slice(h2o_mem_pool_t *pool,
                                           h2o_string_slice slice) {
  h2o_json_clear_error();

  u64 pos = 0;
  skip_whitespace(slice, &pos);

  if (pos >= slice.len) {
    set_error(pool, "Empty JSON string.", pos);
    return null;
  }

  h2o_json_node_t *node = null;
  char first_char = slice.base[pos];

  switch (first_char) {
  case '{':
    node = parse_object_full(pool, slice, &pos);
    break;
  case '[':
    node = parse_array_full(pool, slice, &pos);
    break;
  default:
    node = parse_value_full(pool, slice, &pos);
    break;
  }

  if (!node && !global_json_error.has_error) {
    set_error(pool, "Failed to parse JSON.", pos);
    return null;
  }

  skip_whitespace(slice, &pos);
  if (pos < slice.len) {
    set_error(pool, "Trailing characters after JSON.", pos);
    return null;
  }

  return node;
}

//
//
//

h2o_json_node_t *h2o_json_get_object_slice(h2o_mem_pool_t *pool,
                                           h2o_json_node_t *obj,
                                           h2o_string_slice key) {
  if (!h2o_json_is_object(obj))
    return null;

  h2o_string_slice temp_key = (h2o_string_slice){
      .base = key.base,
      .len = key.len,
  };

  if (obj->data.object_data.is_lazy) {
    struct {
      u64 pos;
      h2o_json_type_t type;
    } *lazy_info = h2o_sm_get(obj->data.object_data.lazy.lazy_poses, &temp_key);

    if (!lazy_info)
      return null;

    u64 current_pos = lazy_info->pos;
    h2o_string_slice value_json = (h2o_string_slice){
        .base = obj->slice.base + lazy_info->pos,
        .len = obj->slice.len - lazy_info->pos,
    };

    h2o_json_node_t *node = null;

    switch (lazy_info->type) {
    case H2O_JSON_OBJECT:
      node = parse_object_lazy(
          pool,
          (h2o_string_slice){.base = value_json.base, .len = value_json.len},
          &current_pos);
      break;

    case H2O_JSON_ARRAY:
      node = parse_array_lazy(
          pool,
          (h2o_string_slice){.base = value_json.base, .len = value_json.len},
          &current_pos);
      break;

    case H2O_JSON_STRING: {
      node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
      node->type = H2O_JSON_STRING;
      node->slice =
          (h2o_string_slice){.base = value_json.base, .len = value_json.len};

      if (value_json.len > 1 && value_json.base[0] == '"')
        node->data.str_data.str_val = (h2o_string_slice){
            .base = value_json.base + 1, .len = value_json.len - 2};
    } break;
    case H2O_JSON_INTEGER:
    case H2O_JSON_BOOL:
    case H2O_JSON_FLOAT:
      node = parse_primitive_full(
          pool,
          (h2o_string_slice){.base = value_json.base, .len = value_json.len},
          &current_pos);
      break;
    case H2O_JSON_NULL:
      node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
      node->type = H2O_JSON_NULL;
      node->slice =
          (h2o_string_slice){.base = value_json.base, .len = value_json.len};
      break;

    default:
      node = parse_value_full(
          pool,
          (h2o_string_slice){.base = value_json.base, .len = value_json.len},
          &current_pos);
      break;
    }
    return node;
  } else
    return h2o_sm_get(obj->data.object_data.full.nodes, &temp_key);
}

h2o_json_node_t *h2o_json_get_object(h2o_mem_pool_t *pool, h2o_json_node_t *obj,
                                     h2o_string *key) {
  h2o_string_slice slice = h2o_string_slice_from_h2o_string(key);

  return h2o_json_get_object_slice(pool, obj, slice);
}

h2o_json_node_t *h2o_json_get_array(h2o_mem_pool_t *pool, h2o_json_node_t *arr,
                                    u64 idx) {
  if (!h2o_json_is_array(arr))
    return null;

  if (arr->data.array_data.is_lazy) {
    if (idx >= arr->data.array_data.lazy.count)
      return null;
    u64 element_pos = arr->data.array_data.lazy.poses[idx];

    h2o_string_slice element_slice = (h2o_string_slice){
        .base = arr->slice.base + element_pos,
        .len = arr->slice.len - element_pos,
    };

    u64 current_pos = 0;
    return parse_value_lazy(pool, element_slice, &current_pos);
  }

  if (idx >= h2o_da_len(arr->data.array_data.full.items))
    return null;
  return arr->data.array_data.full.items[idx];
}

h2o_json_node_t *h2o_json_path_get(h2o_mem_pool_t *pool,
                                   const h2o_string *json_string,
                                   const h2o_string *path) {
  h2o_string_slice json_slice = h2o_string_slice_from_h2o_string(json_string);
  h2o_string_slice path_slice = h2o_string_slice_from_h2o_string(path);

  return h2o_json_path_get_slice(pool, json_slice, path_slice);
}

h2o_json_node_t *h2o_json_path_get_slice(h2o_mem_pool_t *pool,
                                         const h2o_string_slice json_slice,
                                         const h2o_string_slice path_slice) {
  h2o_json_node_t *root = h2o_json_parse_from_lazy_slice(pool, json_slice);
  if (!root)
    return null;

  h2o_string_slice remaining = path_slice;
  h2o_json_node_t *current_node = root;

  while (remaining.len > 0 && current_node) {
    if (remaining.base[0] == '.')
      remaining = h2o_string_slice_slice(remaining, 1, remaining.len);

    i64 next_dot = h2o_string_slice_findc(remaining, '.');
    i64 next_bracket = h2o_string_slice_findc(remaining, '[');

    if (next_bracket != -1 && (next_dot == -1 || next_bracket < next_dot)) {
      h2o_string_slice key = remaining;

      if (next_bracket > 0) {
        key = h2o_string_slice_slice(remaining, 0, next_bracket);
        if (key.len > 0) {
          if (!h2o_json_is_object(current_node)) {
            set_error(pool, "Expected object for key.", 0);
            return null;
          }

          current_node = h2o_json_get_object_slice(pool, current_node, key);
        }
      } else {
        if (!h2o_json_is_array(current_node)) {
          set_error(pool, "Expected array for direct array access.", 0);
          return null;
        }

        key = (h2o_string_slice){0};
      }

      i64 closing_bracket =
          h2o_string_slice_findc_from(remaining, next_bracket + 1, ']');
      if (closing_bracket <= next_bracket) {
        set_error(pool, "Expected closing bracket.", 0);
        return null;
      }

      h2o_string_slice idx_slice =
          h2o_string_slice_slice(remaining, next_bracket + 1, closing_bracket);
      cstr idx_buf[21];
      if (idx_slice.len < 21) {
        memcpy(idx_buf, idx_slice.base, idx_slice.len);
        idx_buf[idx_slice.len] = '\0';

        cstr *endptr = null;
        u64 idx = strtoull(idx_buf, &endptr, 10);
        if (endptr == idx_buf) {
          set_error(pool, "Failed to parse array index.", 0);
          return null;
        }

        if (!current_node || !h2o_json_is_array(current_node)) {
          set_error(pool, "Expected array for array index.", 0);
          return null;
        }

        current_node = h2o_json_get_array(pool, current_node, idx);
        if (!current_node) {
          set_error(pool, "Failed to get array index.", 0);
          return null;
        }
      } else {
        set_error(pool, "Array index too large.", 0);
        return null;
      }

      remaining =
          h2o_string_slice_slice(remaining, closing_bracket + 1, remaining.len);
    } else if (next_dot != -1) {
      h2o_string_slice key = h2o_string_slice_slice(remaining, 0, next_dot);

      if (!h2o_json_is_object(current_node)) {
        set_error(pool, "Expected object for key.", 0);
        return null;
      }

      current_node = h2o_json_get_object_slice(pool, current_node, key);
      remaining =
          h2o_string_slice_slice(remaining, next_dot + 1, remaining.len);
    } else {
      if (!h2o_json_is_object(current_node)) {
        set_error(pool, "Expected object for key.", 0);
        return null;
      }

      current_node = h2o_json_get_object_slice(pool, current_node, remaining);
      remaining = (h2o_string_slice){0};
    }
  }

  return current_node;
}

h2o_json_node_t *h2o_json_parse_from_lazy_slice(h2o_mem_pool_t *pool,
                                                h2o_string_slice slice) {
  h2o_json_clear_error();

  u64 pos = 0;
  skip_whitespace(slice, &pos);

  if (pos >= slice.len) {
    set_error(pool, "Empty JSON string.", pos);
    return null;
  }

  h2o_json_node_t *node = null;
  char first_char = slice.base[pos];

  switch (first_char) {
  case '{':
    node = parse_object_lazy(
        pool, (h2o_string_slice){.base = slice.base, .len = slice.len}, &pos);
    break;
  case '[':
    node = parse_array_lazy(
        pool, (h2o_string_slice){.base = slice.base, .len = slice.len}, &pos);
    break;
  default:
    node = parse_value_lazy(
        pool, (h2o_string_slice){.base = slice.base, .len = slice.len}, &pos);
    break;
  }

  if (!node && !global_json_error.has_error) {
    set_error(pool, "Failed to parse JSON.", pos);
    return null;
  }

  skip_whitespace(slice, &pos);
  if (pos < slice.len && node && node->type != H2O_JSON_INVALID) {
    set_error(pool, "Trailing characters after JSON.", pos);
    return null;
  }

  return node;
}

//
//
//

b32 h2o_json_is_valid(h2o_json_node_t *node) {
  return node && node->type != H2O_JSON_INVALID;
}

b32 h2o_json_is_null(h2o_json_node_t *node) {
  return h2o_json_is_valid(node) && node->type == H2O_JSON_NULL;
}

b32 h2o_json_is_bool(h2o_json_node_t *node) {
  return h2o_json_is_valid(node) && node->type == H2O_JSON_BOOL;
}

b32 h2o_json_is_integer(h2o_json_node_t *node) {
  return h2o_json_is_valid(node) && node->type == H2O_JSON_INTEGER;
}

b32 h2o_json_is_float(h2o_json_node_t *node) {
  return h2o_json_is_valid(node) && node->type == H2O_JSON_FLOAT;
}

b32 h2o_json_is_string(h2o_json_node_t *node) {
  return h2o_json_is_valid(node) && node->type == H2O_JSON_STRING;
}

b32 h2o_json_is_array(h2o_json_node_t *node) {
  return h2o_json_is_valid(node) && node->type == H2O_JSON_ARRAY;
}

b32 h2o_json_is_object(h2o_json_node_t *node) {
  return h2o_json_is_valid(node) && node->type == H2O_JSON_OBJECT;
}

//
//
//

b32 h2o_json_get_bool(h2o_json_node_t *node) {
  if (!h2o_json_is_bool(node))
    return false;
  return node->data.bool_data.bool_val;
}

i64 h2o_json_get_integer(h2o_json_node_t *node) {
  if (!h2o_json_is_integer(node))
    return 0;
  return node->data.int_data.int_val;
}

f64 h2o_json_get_float(h2o_json_node_t *node) {
  if (!h2o_json_is_float(node))
    return NAN;
  return node->data.float_data.float_val;
}

h2o_string_slice h2o_json_get_string(h2o_json_node_t *node) {
  if (!h2o_json_is_string(node))
    return (h2o_string_slice){0};
  return node->data.str_data.str_val;
}

u64 h2o_json_get_array_count(h2o_json_node_t *node) {
  if (!h2o_json_is_array(node))
    return 0;

  if (node->data.array_data.is_lazy)
    return node->data.array_data.lazy.count;

  return h2o_da_len(node->data.array_data.full.items);
}

//
//
//

h2o_json_node_t *h2o_json_new_null(h2o_mem_pool_t *pool) {
  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  *node = (h2o_json_node_t){0};

  node->type = H2O_JSON_NULL;
  node->slice = (h2o_string_slice){
      .base = "null",
      .len = 4,
  };

  return node;
}

h2o_json_node_t *h2o_json_new_bool(h2o_mem_pool_t *pool, b32 val) {
  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  *node = (h2o_json_node_t){0};

  node->type = H2O_JSON_BOOL;
  node->data.bool_data.bool_val = val;
  node->slice = (h2o_string_slice){
      .base = val ? "true" : "false",
      .len = val ? 4 : 5,
  };

  return node;
}

h2o_json_node_t *h2o_json_new_integer(h2o_mem_pool_t *pool, i64 val) {
  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  *node = (h2o_json_node_t){0};

  node->type = H2O_JSON_INTEGER;
  node->data.int_data.int_val = val;

  cstr buf[21];
  i64 len = snprintf(buf, 21, "%lld", (long long)val);

  cstr *str_data = h2o_mem_alloc_pool(pool, cstr, len + 1);
  memcpy(str_data, buf, len);

  node->slice = (h2o_string_slice){
      .base = str_data,
      .len = len,
  };

  return node;
}

h2o_json_node_t *h2o_json_new_float(h2o_mem_pool_t *pool, f64 val) {
  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  *node = (h2o_json_node_t){0};
  node->type = H2O_JSON_FLOAT;
  node->data.float_data.float_val = val;

  cstr buf[256];
  i64 len = snprintf(buf, 256, "%g", val);

  cstr *str_data = h2o_mem_alloc_pool(pool, cstr, len + 1);
  memcpy(str_data, buf, len);
  str_data[len] = '\0';

  node->slice = (h2o_string_slice){
      .base = str_data,
      .len = len,
  };

  return node;
}

h2o_json_node_t *h2o_json_new_string(h2o_mem_pool_t *pool, h2o_string *val) {
  h2o_string_slice slice = h2o_string_slice_from_h2o_string(val);

  return h2o_json_new_string_slice(pool, slice);
}

h2o_json_node_t *h2o_json_new_string_slice(h2o_mem_pool_t *pool,
                                           h2o_string_slice val) {
  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  *node = (h2o_json_node_t){0};
  node->type = H2O_JSON_STRING;
  node->data.str_data.str_val = val;

  cstr *str_data = h2o_mem_alloc_pool(pool, cstr, val.len + 3);
  str_data[0] = '"';
  memcpy(str_data + 1, val.base, val.len);
  str_data[val.len + 1] = '"';

  node->slice = (h2o_string_slice){
      .base = str_data,
      .len = val.len + 2,
  };

  return node;
}

h2o_json_node_t *h2o_json_new_array(h2o_mem_pool_t *pool) {
  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  *node = (h2o_json_node_t){0};

  node->type = H2O_JSON_ARRAY;
  h2o_da_new(pool, node->data.array_data.full.items, 16);
  node->slice = (h2o_string_slice){
      .base = "[",
      .len = 1,
  };

  return node;
}

h2o_json_node_t *h2o_json_new_object(h2o_mem_pool_t *pool) {
  h2o_json_node_t *node = h2o_mem_alloc_pool(pool, h2o_json_node_t, 1);
  *node = (h2o_json_node_t){0};
  node->type = H2O_JSON_OBJECT;
  node->data.object_data.full.nodes = h2o_sm_new(pool, 16);
  node->data.object_data.keys = h2o_sm_new(pool, 16);
  node->slice = (h2o_string_slice){
      .base = "{",
      .len = 1,
  };

  return node;
}

//
//
//

b32 h2o_json_array_push(h2o_mem_pool_t *pool, h2o_json_node_t *arr,
                        h2o_json_node_t *val) {
  if (!h2o_json_is_array(arr))
    return false;

  if (!arr->data.array_data.full.items)
    h2o_da_new(pool, arr->data.array_data.full.items, 16);

  h2o_da_append(pool, arr->data.array_data.full.items, val);
  return true;
}

b32 h2o_json_array_set(h2o_mem_pool_t *pool, h2o_json_node_t *arr,
                       h2o_json_node_t *val, u64 idx) {
  if (!h2o_json_is_array(arr))
    return false;

  if (!arr->data.array_data.full.items)
    h2o_da_new(pool, arr->data.array_data.full.items, 16);

  if (idx >= h2o_da_len(arr->data.array_data.full.items))
    return false;

  arr->data.array_data.full.items[idx] = val;
  return true;
}

b32 h2o_json_obj_set(h2o_mem_pool_t *pool, h2o_json_node_t *obj,
                     h2o_string *key, h2o_json_node_t *val) {
  if (!h2o_json_is_object(obj))
    return false;

  h2o_sm_insert(obj->data.object_data.keys, key, key);
  h2o_sm_insert(obj->data.object_data.full.nodes, key, val);

  return true;
}

//
//
//

h2o_string *h2o_json_write(h2o_mem_pool_t *pool, h2o_json_node_t *node,
                           h2o_json_write_options_t options, u64 indent) {

  u64 estimated_size = estimate_json_size(node);

  if (options & H2O_JSON_WRITE_PRETTY) {
    estimated_size *= 2;
  }

#define max(a, b) ((a) > (b) ? (a) : (b))
  estimated_size = max(estimated_size, 256);
#undef max

  h2o_string *result = h2o_string_new(pool, estimated_size);
  result->base[0] = '\0';
  result->len = 0;

  b32 pretty = (options & H2O_JSON_WRITE_PRETTY) != 0;

  write_json_value(pool, result, node, indent, pretty);

  return result;
}

h2o_string *h2o_json_write_pretty(h2o_mem_pool_t *pool, h2o_json_node_t *node,
                                  u64 indent) {
  return h2o_json_write(
      pool, node, H2O_JSON_WRITE_PRETTY | H2O_JSON_WRITE_SORT_KEYS, indent);
}
