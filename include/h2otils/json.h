#ifndef H2OTILS_JSON_H
#define H2OTILS_JSON_H

#include <h2o/memory.h>

//
// TODO: Document this.
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

typedef enum h2o_json_type {
  H2O_JSON_NULL,
  H2O_JSON_BOOL,
  H2O_JSON_FLOAT,
  H2O_JSON_INTEGER,
  H2O_JSON_STRING,
  H2O_JSON_ARRAY,
  H2O_JSON_OBJECT,
  H2O_JSON_INVALID,
} h2o_json_type_t;

typedef enum h2o_json_parse_method {
  H2O_JSON_PARSE_FULL,
  H2O_JSON_PARSE_LAZY,
} h2o_json_parse_method_t;

typedef struct h2o_json_node {
  h2o_json_type_t type;
  h2o_string_slice slice;

  union {
    struct {
      b32 bool_val;
    } bool_data;

    struct {
      i64 int_val;
    } int_data;

    struct {
      f64 float_val;
    } float_data;

    struct {
      h2o_string_slice str_val;
    } str_data;

    struct {
      struct {
        struct h2o_json_node **items; // Using darray.h
      } full;
      struct {
        u64 *poses;
        u64 count;
      } lazy;
      b32 is_lazy;
    } array_data;

    struct {
      h2o_stringmap_t *keys;
      struct {
        h2o_stringmap_t *nodes;
      } full;
      struct {
        h2o_stringmap_t *lazy_poses;
      } lazy;
      b32 is_lazy;
    } object_data;
  } data;
} h2o_json_node_t;

//
//
//

h2o_json_node_t *h2o_json_parse(h2o_mem_pool_t *pool,
                                const h2o_string *json_body);
h2o_json_node_t *h2o_json_parse_from_slice(h2o_mem_pool_t *pool,
                                           const h2o_string_slice slice);

h2o_json_node_t *h2o_json_parse_from_lazy_slice(h2o_mem_pool_t *pool,
                                                h2o_string_slice slice);

//
//
//

h2o_json_node_t *h2o_json_get_object(h2o_mem_pool_t *pool, h2o_json_node_t *obj,
                                     h2o_string *key);
h2o_json_node_t *h2o_json_get_object_slice(h2o_mem_pool_t *pool,
                                           h2o_json_node_t *obj,
                                           h2o_string_slice slice);
h2o_json_node_t *h2o_json_get_array(h2o_mem_pool_t *pool, h2o_json_node_t *arr,
                                    u64 idx);

h2o_json_node_t *h2o_json_path_get(h2o_mem_pool_t *pool,
                                   const h2o_string *json_string,
                                   const h2o_string *path);
h2o_json_node_t *h2o_json_path_get_slice(h2o_mem_pool_t *pool,
                                         const h2o_string_slice json_slice,
                                         const h2o_string_slice path);

//
//
//

b32 h2o_json_is_valid(h2o_json_node_t *node);
b32 h2o_json_is_array(h2o_json_node_t *node);
b32 h2o_json_is_object(h2o_json_node_t *node);
b32 h2o_json_is_string(h2o_json_node_t *node);
b32 h2o_json_is_integer(h2o_json_node_t *node);
b32 h2o_json_is_float(h2o_json_node_t *node);
b32 h2o_json_is_bool(h2o_json_node_t *node);
b32 h2o_json_is_null(h2o_json_node_t *node);

//
//
//

b32 h2o_json_get_bool(h2o_json_node_t *node);
i64 h2o_json_get_integer(h2o_json_node_t *node);
f64 h2o_json_get_float(h2o_json_node_t *node);
h2o_string_slice h2o_json_get_string(h2o_json_node_t *node);
u64 h2o_json_get_array_count(h2o_json_node_t *node);

//
//
//

typedef enum h2o_json_write_options {
  H2O_JSON_WRITE_DEFAULT = 0,
  H2O_JSON_WRITE_PRETTY = 1 << 0,
  H2O_JSON_WRITE_SORT_KEYS = 1 << 1,
  H2O_JSON_WRITE_COMPACT = 1 << 2,
} h2o_json_write_options_t;

h2o_string *h2o_json_write(h2o_mem_pool_t *pool, h2o_json_node_t *node,
                           h2o_json_write_options_t options, u64 indent);

h2o_string *h2o_json_write_pretty(h2o_mem_pool_t *pool, h2o_json_node_t *node,
                                  u64 indent);

//
//
//

h2o_json_node_t *h2o_json_new_null(h2o_mem_pool_t *pool);
h2o_json_node_t *h2o_json_new_bool(h2o_mem_pool_t *pool, b32 val);
h2o_json_node_t *h2o_json_new_integer(h2o_mem_pool_t *pool, i64 val);
h2o_json_node_t *h2o_json_new_float(h2o_mem_pool_t *pool, f64 val);
h2o_json_node_t *h2o_json_new_string(h2o_mem_pool_t *pool, h2o_string *val);
h2o_json_node_t *h2o_json_new_string_slice(h2o_mem_pool_t *pool,
                                           h2o_string_slice val);
h2o_json_node_t *h2o_json_new_array(h2o_mem_pool_t *pool);
h2o_json_node_t *h2o_json_new_object(h2o_mem_pool_t *pool);

//
//
//

b32 h2o_json_array_push(h2o_mem_pool_t *pool, h2o_json_node_t *arr,
                        h2o_json_node_t *val);

b32 h2o_json_array_set(h2o_mem_pool_t *pool, h2o_json_node_t *arr,
                       h2o_json_node_t *val, u64 idx);
b32 h2o_json_obj_set(h2o_mem_pool_t *pool, h2o_json_node_t *obj,
                     h2o_string *key, h2o_json_node_t *val);

//
//
//

typedef struct h2o_json_error {
  b32 has_error;
  cstr *msg;
  u64 pos;
} h2o_json_error_t;

h2o_json_error_t *h2o_json_get_error(h2o_mem_pool_t *pool);
void h2o_json_clear_error(void);

#endif // !H2OTILS_JSON_H
