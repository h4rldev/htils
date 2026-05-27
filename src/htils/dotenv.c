#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//
//

#include <htils/assert.h>
#include <htils/darray.h>
#include <htils/dotenv.h>
#include <htils/file.h>
#include <htils/path.h>
#include <htils/string.h>

//
//
//

/**
 * @brief Check if a path is a .env file.
 *
 * @details By first retrieveing its extension, and then verifying that the
 * extension is .env.
 *
 * @param arena The arena to allocate from.
 * @param path The path to check.
 *
 * @pre @c arena and @c path must be valid and cannot be `null`.
 *
 * @return True if the path is a .env file, false if it isn't.
 */
static b32 is_env_file(arena_t *arena, const string *path) {
  string *ext = path_extension(arena, path);
  if (ext == null)
    return false;
  return stringcmp(ext, HTILS_STR(".env"));
}

/**
 * @brief Find the first .env file in a directory.
 *
 * @details Crawling through a directory and return the path of the first known
 * .env file.
 *
 * @param arena The arena to allocate from.
 * @param path The path to find the .env file in.
 *
 * @pre @c arena, and @c path must be valid and cannot be `null`.
 *
 * @return The path to the .env file as \ref string, or null if none was found.
 */
static string *find_first_env_file(arena_t *arena, const string *path) {
  DIR *dir = opendir(string_to_cstr(path));
  if (!dir)
    return null;

  struct dirent *entry;
  while ((entry = readdir(dir)) != null) {
    if ((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
        (entry->d_name[0] == '.' && entry->d_name[1] == '.' &&
         entry->d_name[2] == '\0'))
      continue;

    string *full_path = path_join(arena, path, HTILS_STR(entry->d_name));

    if (entry->d_type == DT_DIR)
      find_first_env_file(arena, full_path);
    else {
      if (is_env_file(arena, full_path)) {
        closedir(dir);
        return full_path;
      }
    }
  }

  closedir(dir);
  return null;
}

/**
 * @brief Convert a string to uppercase.
 *
 * @details By iterating through the string, and converting each character to
 * it's uppercase counterpart.
 *
 * @param str The string to convert.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
static void to_upper(string *str) {
  for (u64 i = 0; i < str->len; i++)
    str->base[i] = toupper(str->base[i]);
}

/**
 * @brief Trim quotes from a string.
 *
 * @details By checking if the first and last characters are quotes, and then
 * trims and memmoves as needed.
 *
 * @param str The string to trim.
 *
 * @pre @c str must be valid and cannot be `null`.
 */
static void trim_quotes(string *str) {
  htils_assert(str != null && "String cannot be null.");
  htils_assert(str->len > 0 && "String cannot be empty.");
  htils_assert(str->base[0] == '"' && "String must start with a quote");

  if (str->len < 2)
    return;

  if ((str->base[0] == '"' && str->base[str->len - 1] == '"') ||
      (str->base[0] == '\'' && str->base[str->len] == '\'')) {
    memmove(str->base, str->base + 1, str->len - 1);
  }

  str->len -= 2;
}

/**
 * @brief Parse a line of a .env file.
 *
 * @details By checking if the line is empty, a comment, or a new_line before
 * trimming whitespace, making sure the key is upper case, and trimming quotes
 * from the value, then setting the key and line to the parsed values.
 *
 * @param arena The arena to allocate from.
 * @param line The line to parse.
 * @param key The key of the line.
 * @param value The value of the line.
 *
 * @pre
 * - @c arena, @c line, must be valid and cannot be `null`.
 * - @c key and @c value must be valid pointers that you can overwrite.
 *
 * @return True if the line was parsed, false if it wasn't.
 */
static b32 parse_line(arena_t *arena, string *line, string **key,
                      string **value) {
  if (!line)
    return false;

  if (line->len == 0)
    return false;

  if (line->base[0] == '#' || line->base[0] == ';')
    return false;

  if (line->base[0] == '\n' || line->base[0] == '\r')
    return false;

  i64 idx = string_findc(line, '=');
  string *key_part = string_new(arena, idx);
  string *value_part = string_new(arena, line->len - idx - 1);

  memcpy(key_part->base, line->base, idx);
  memcpy(value_part->base, line->base + idx + 1, line->len - idx - 1);

  string_trim(key_part);
  string_trim(value_part);

  to_upper(key_part);
  trim_quotes(value_part);

  *key = key_part;
  *value = value_part;

  return true;
}

//
//
//

i32 htils_dotenv_load(arena_t *arena, const string *path) {
  htils_assert(path && "Path is null.");
  htils_assert(path->len > 0 && "Path is empty.");

  string *env_path;
  i32 matches = 0;

  if (!does_path_exist(path)) {
    fprintf(stderr, "Path does not exist: %s\n", string_to_cstr(path));
    return -1;
  }

  string *basename = path_basename(arena, path);
  if (basename && stringcmp(basename, HTILS_STR(".env")))
    env_path = (string *)path;
  else if (path_extension(arena, path) == null)
    env_path = find_first_env_file(arena, path);
  else
    env_path = (string *)path; // Path doesn't have a .env or anything, but
                               // we'll read it anyways.

  string *contents = read_file(arena, env_path);
  string **lines;
  da_new(arena, lines, 2);

  string_split(contents, '\n', &lines, arena);
  for (u64 i = 0; i < da_len(lines); i++) {
    string *line = lines[i];
    string *key = null;
    string *value = null;

    if (parse_line(arena, line, &key, &value) == false)
      continue;

    if (setenv(string_to_cstr(key), string_to_cstr(value), 1) != 0) {
      fprintf(stderr, "Failed to set env var: %s\n", string_to_cstr(key));
      return -1;
    }

    matches++;
  }

  return matches;
}
