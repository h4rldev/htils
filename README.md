# htils

A set of utilities for C, primarily targeting Linux, though most (if not all) of
them should work on Windows too. Part of the `htils` `bread` `butter` `cheese`
stack.

## Table of Contents

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Features](#features)
- [Files](#files)
  - [htils](#htils)
- [Building](#building)
  - [Prerequisites](#prerequisites)
  - [How to build](#how-to-build)
- [Todos](#todos)
- [License](#license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Features

- Basic type aliases I prefer over the built-ins: `u8`-`u64`, `i8`-`i64`, `b32`,
  `cstr`, `ucstr`, `null`.
- A small, flexible reserve-and-commit arena allocator.
- A string type over unsigned 8-bit integers, with lots of manipulation helpers.
- An assert that calls `exit()` instead of `abort()`.
- A simple, not-particularly-type-agnostic string map.
- Path manipulation.
- Type-agnostic dynamic arrays.
- File IO.
- A small CLI parser.
- A `.env` parser.
- A worker pool / task system (thread-safe builds only).
- A tiny test framework.

The library is documented in the headers under `./include`, rendered as a
documentation site (see [Building](#building)), and deployed at
<https://htils.h4rl.dev>.

## Files

### htils

> Arena allocator

`htils/arena.h` for documentation and exports; `htils/arena.c` for the
implementation.

> Strings

`htils/string.h` / `htils/string.c`.

> Assert

`htils/assert.h` / `htils/assert.c`.

> String map

`htils/stringmap.h` / `htils/stringmap.c`.

> Path manipulation

`htils/path.h` / `htils/path.c`.

> Dynamic arrays

`htils/darray.h` (header-only).

> File IO

`htils/file.h` / `htils/file.c`.

> CLI parser

`htils/cli.h` / `htils/cli.c`.

> `.env` parser

`htils/dotenv.h` / `htils/dotenv.c`.

> Workers

`htils/worker.h` / `htils/worker.c` (thread-safe builds only).

> Test framework

`htils/test.h` (header-only).

## Building

### Prerequisites

- GCC with glibc (the assert relies on GNU extensions)
- [conjure](https://codeberg.org/h4rl/conjure)

### How to build

```
conjure build -p htils-release          # libhtils.so
conjure build -p htils-release-static   # libhtils.a
conjure build -p htils-debug            # libhtils-debug.a
```

Thread-safe builds define `HTILS_THREAD_SAFE` and use the `-threadsafe`
profiles:

```
conjure build -p htils-release-threadsafe
conjure build -p htils-debug-threadsafe
```

The documentation site is generated with `doxygen` into `./doc/html`.

## Todos

- [x] Nix package.
- [ ] Examples.
- [ ] More features.

## License

This project is licensed under the BSD-3 Clause License - see the
[LICENSE](LICENSE) file for details.
