#include <assert.h>
#include <stdio.h>

//
//
//

#include <htils/assert.h>
#include <htils/basictypes.h>
#include <htils/file.h>
#include <htils/string.h>

//
//
//

u64 file_size_stream(FILE *stream) {
  htils_assert(stream && "Stream cannot be null.");

  struct stat st;
  int fd = fileno(stream);
  int res = fstat(fd, &st);
  htils_assert(res == 0 && "Failed to get size of file.");

  return (u64)st.st_size;
}

u64 file_size(const string *path) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");

  const cstr *path_cstr = string_to_cstr(path);
  FILE *stream = fopen(path_cstr, "rb");
  assert(stream != null && "Failed to open file.");

  u64 size = file_size_stream(stream);
  fclose(stream);

  return size;
}

//
//
//

string *read_file_from_stream(arena_t *arena, FILE *stream) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(stream && "Stream cannot be null.");

  u64 size = file_size_stream(stream);
  string *out = string_new(arena, size);

  out->len = fread(out->base, 1, size, stream);
  assert(out->len == size && "Incomplete read from stream, exiting.");

  return out;
}

string *read_file(arena_t *arena, const string *path) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");

  const cstr *path_cstr = string_to_cstr(path);
  FILE *stream = fopen(path_cstr, "rb");
  assert(stream != null && "Failed to open file.");

  string *out = read_file_from_stream(arena, stream);
  fclose(stream);
  return out;
}

//
//
//

string *read_file_from_stream_bytes(arena_t *arena, FILE *stream,
                                    const u64 bytes) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(stream && "Stream cannot be null.");
  htils_assert(bytes > 0 && "Bytes must be greater than 0.");

  string *out = string_new(arena, bytes);

  out->len = fread(out->base, 1, bytes, stream);
  assert(out->len == bytes && "Incomplete read from stream, exiting.");

  return out;
}

string *read_file_bytes(arena_t *arena, const string *path, const u64 bytes) {
  htils_assert(arena && "Arena cannot be null.");
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");
  htils_assert(bytes > 0 && "Bytes must be greater than 0.");

  const cstr *path_cstr = string_to_cstr(path);
  FILE *stream = fopen(path_cstr, "rb");
  assert(stream != null && "Failed to open file.");

  string *out = read_file_from_stream_bytes(arena, stream, bytes);
  fclose(stream);
  return out;
}

//
//
//

u64 write_to_file_stream(FILE *stream, const string *contents) {
  htils_assert(stream && "Stream cannot be null.");
  htils_assert(contents && "Contents cannot be null.");
  htils_assert(contents->len > 0 && "Contents cannot be empty.");

  u64 written = fwrite(contents->base, 1, contents->len, stream);
  assert(written == contents->len && "Failed to write to stream.");

  return written;
}

u64 write_to_file(const string *path, const string *contents) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(contents && "Contents cannot be null.");
  htils_assert(contents->len > 0 && "Contents cannot be empty.");

  const cstr *path_cstr = string_to_cstr(path);
  FILE *stream = fopen(path_cstr, "wb");
  assert(stream != null && "Failed to open file.");

  u64 written = write_to_file_stream(stream, contents);
  fclose(stream);

  return written;
}

//
//
//

u64 write_to_file_stream_bytes(FILE *stream, const string *contents,
                               const u64 bytes) {
  htils_assert(stream && "Stream cannot be null.");
  htils_assert(contents && "Contents cannot be null.");
  htils_assert(bytes > 0 && "Bytes must be greater than 0.");
  htils_assert(contents->len > 0 && "Contents cannot be empty.");

  u64 written = fwrite(contents->base, 1, bytes, stream);
  assert(written == bytes && "Failed to write to stream.");

  return written;
}

u64 write_to_file_bytes(const string *path, const string *contents,
                        const u64 bytes) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(contents && "Contents cannot be null.");
  htils_assert(bytes > 0 && "Bytes must be greater than 0.");
  htils_assert(contents->len > 0 && "Contents cannot be empty.");

  const cstr *path_cstr = string_to_cstr(path);
  FILE *stream = fopen(path_cstr, "wb");
  assert(stream != null && "Failed to open file.");

  u64 written = write_to_file_stream_bytes(stream, contents, bytes);
  fclose(stream);

  return written;
}
