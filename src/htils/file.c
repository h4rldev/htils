#include <assert.h>
#include <stdio.h>

//
//
//

#include <htils/assert.h>
#include <htils/basictypes.h>
#include <htils/file.h>
#include <htils/string.h>

#if defined(_WIN32)
#include <windows.h>

#define fseek64 _fseeki64
#define ftell64 _ftelli64
#elif defined(__linux__)
#include <sys/stat.h>
#include <unistd.h>

#define fseek64 fseeko
#define ftell64 ftello
#else
#error "Unsupported platform."
#endif

//
//
//

u64 file_size_stream(FILE *stream) {
  htils_assert(stream && "Stream cannot be null.");

  int res = fseek64(stream, 0, SEEK_END);
  htils_assert(res == 0 && "Failed to seek to end of file, Might be a pipe, "
                           "terminal, or something else.");

  i64 size = ftell64(stream);
  assert(size >= 0 && "Failed to get size of file.");

  rewind(stream);
  return (u64)size;
}

u64 file_size(const string *path) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");

  FILE *stream = fopen(string_to_cstr(path), "rb");
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

  FILE *stream = fopen(string_to_cstr(path), "rb");
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

  FILE *stream = fopen(string_to_cstr(path), "rb");
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

  FILE *stream = fopen(string_to_cstr(path), "wb");
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

  FILE *stream = fopen(string_to_cstr(path), "wb");
  assert(stream != null && "Failed to open file.");

  u64 written = write_to_file_stream_bytes(stream, contents, bytes);
  fclose(stream);

  return written;
}

//
//
//

b32 make_dir(const string *path) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");

#if defined(__linux__)
  int res = mkdir(string_to_cstr(path), 0755);
  return res == 0;
#elif defined(_WIN32)
  int res = CreateDirectory(string_to_cstr(path), null);
  return res != 0;
#else
#error "Unsupported platform."
#endif
}

b32 does_path_exist(const string *path) {
  htils_assert(path && "Path cannot be null.");
  htils_assert(path->len > 0 && "Path cannot be empty.");

#if defined(__linux__)
  struct stat sb;
  return stat(string_to_cstr(path), &sb) == 0;
#elif defined(_WIN32)
  DWORD attrs = GetFileAttributes(string_to_cstr(path));
  return attrs != INVALID_FILE_ATTRIBUTES;
#else
#error "Unsupported platform."
#endif
}
