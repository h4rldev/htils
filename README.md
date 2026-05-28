# htils & h2otils

A set of utilities I prefer for C for primarily Linux, but I'm pretty certain most if not all the utilities should work for Windows too.

## Table of Contents

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Features](#features)
  - [htils](#htils)
  - [h2otils](#h2otils)
- [Files](#files)
  - [htils](#htils-1)
  - [h2otils](#h2otils-1)
- [Building](#building)
  - [Prerequisites](#prerequisites)
  - [How to build](#how-to-build)
  - [For debug builds](#for-debug-builds)
- [Todos](#todos)
- [License](#license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Features

### htils

- Basic type-aliases that I prefer over the built-in types.
  - `u8`-`u64`
  - `i8`-`i64`
  - `b32`
  - `cstr`
  - `ucstr`
  - `null`

- A small and flexible reserve and commit arena allocator.
- A string type based on unsigned 8bit integers, with tons of features for its manipulation.
- A custom assert that uses exit() instead of abort().
- A simple stringmap, that isn't super type-agnostic, but it works well enough.
- Path manipulation.
- Type-agnostic dynamic arrays.
- File IO.
- An easy to use cli parser.
- A dotenv parser.
- A tiny test framework.

> Final note in the features tab
>
> This library is mostly documented, so if you need help on how to use each respective api, read the header files in `./include`
>
> If you want a nice little documentation site, run doxygen in a clone of this repo and open index.html in `./doc/html`
>
> Edit: The documentation site is now deployed to [https://htils.h4rl.dev](https://htils.h4rl.dev)!

### h2otils

- Cookie generation.
- String manipulation based on h2o_iovec_t.
- h2o Memory Pool based dynamic arrays
- Variant of the htils test framework for h2o.
- More to come...

## Files

### htils

> Arena allocator

htils/arena.h for documentation and function exports.

htils/arena.c for implementation.

> Strings

htils/string.h for documentation and function exports.

htils/string.c for implementation.

> Assert

htils/assert.h for documentation and function exports.

htils/assert.c for implementation.

> Stringmap

htils/stringmap.h for documentation and function exports.

htils/stringmap.c for implementation.

> Path manipulation

htils/path.h for documentation and function exports.

htils/path.c for implementation.

> Dynamic arrays

htils/darray.h for documentation and implementation.

> File IO

htils/file.h for documentation and function exports.

htils/file.c for implementation.

> Cli parser

htils/cli.h for documentation and function exports.

htils/cli.c for implementation.

> .env parser

htils/dotenv.h for documentation and function exports.

htils/dotenv.c for implementation.

> Test framework

htils/test.h for documentation, and implementation.

### h2otils

> Cookie generation

h2otils/cookie.h for documentation and function exports.

h2otils/cookie.c for implementation (WIP).

> Dynamic arrays

h2otils/darray.h for documentation and implementation (WIP).

> String manipulation

h2otils/string.h for documentation and function exports.

h2otils/string.c for implementation (WIP).

> Test framework

h2otils/test.h for documentation and implementation (WIP).

## Building

### Prerequisites

- GCC with GLIBC (assert depends on GNU extensions)
- just
- mold

### How to build

> Run

```
just release
```

### For debug builds

> Run

```
just debug
```

## Todos

- [x] - Nix package.
- [ ] - Finish h2otils.
- [ ] - Examples.
- [ ] - More features.

## License

This project is licensed under the BSD-3 Clause License - see the [LICENSE](LICENSE) file for details.
