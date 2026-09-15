# htils

A set of small utilities for C, primarily targeting Linux. This page is the API
overview; each header under `include/htils` carries the full documentation, and
the project README covers building.

## Arena allocator (`htils/arena.h`)

\ref arena_t is a reserve-and-commit arena allocator. Allocate with \ref
arena_alloc or \ref arena_alloc_zeroed, roll back to a saved position with \ref
arena_dealloc_to, and reclaim the whole arena with \ref arena_clear; \ref
arena_free releases the backing memory.

\ref temp_arena_t is a scoped child of an arena: it snapshots the parent on \ref
temp_arena_new and restores it on \ref temp_arena_free.

## Strings (`htils/string.h`)

\ref string is a growable byte string; \ref string_slice is a non-owning view
over one. Concatenate with \ref string_concat, \ref string_concatb and \ref
string_concatf, compare with \ref stringcmp and \ref stringcmpb, search with
\ref string_findc and \ref string_find_sstr, split with \ref string_split, and
trim with \ref string_trim / \ref string_trim_left / \ref string_trim_right.

## Assert (`htils/assert.h`)

\ref htils_assert reports the failed expression and exits, instead of aborting.

## String map (`htils/stringmap.h`)

\ref stringmap_t is a string-keyed map. Insert with \ref sm_insert and remove
with \ref sm_kill; operations return a \ref stringmap_result_t.

## Paths (`htils/path.h`)

Filesystem path helpers: \ref does_path_exist, \ref make_dir and \ref
path_remove.

## Dynamic arrays (`htils/darray.h`)

Header-only, type-agnostic dynamic arrays. Create one with \ref da_new, append
with \ref da_append, and inspect or trim with \ref da_len, \ref da_cap, \ref
da_last, \ref da_pop and \ref da_clear.

## File IO (`htils/file.h`)

Read a whole file with \ref read_file (or \ref read_file_bytes for a prefix) and
write with \ref write_to_file / \ref write_to_file_bytes; the `*_stream`
variants take a `FILE *`.

## CLI (`htils/cli.h`)

\ref htils_cli_t is a small argument parser: register options with \ref cli_add
and parse with \ref parse_cli.

## dotenv (`htils/dotenv.h`)

\ref htils_dotenv_load parses a `.env` file.

## Workers (`htils/worker.h`)

Thread-safe builds only. \ref htils_worker_t runs tasks off the main thread:
configure with \ref htils_worker_config_t (\ref htils_worker_config_default),
then \ref htils_worker_spawn for a single-shot task or \ref
htils_worker_spawn_task for a sliced one. Tasks can poll \ref
htils_worker_should_stop and check \ref htils_worker_paused.

## Tests (`htils/test.h`)

A tiny test framework: declare a test with \ref HTILS_TEST, run it with \ref
HTILS_TEST_RUN, assert with \ref HTILS_TEST_ASSERT, and finish with \ref
HTILS_TEST_RESULT.
