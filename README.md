# htils

A set of utilities I prefer for C for primarily Linux, but I'm pretty certain most if not all the utilities should work for Windows too.

## Table of Contents

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Features](#features)
- [Building](#building)
  - [Prerequisites](#prerequisites)
  - [How to build](#how-to-build)
  - [For debug builds](#for-debug-builds)
- [Todos](#todos)
- [License](#license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Features

- Basic type-aliases that I prefer over the built-in types.
  - `u8`-`u64`
  - `i8`-`i64`
  - `b32`
  - `cstr`
  - `ucstr`
  - `null`
- A small and flexible reserve and commit arena.
  > Available through arena_new, arena_alloc, etc.
- A string type based on unsigned 8bit integers, with tons of features for its manipulation.
  > Available through htils/string.h
- A custom assert that uses exit() instead of abort().
  > Available through htils/assert.h
- A simple hashmap, that isn't super type-agnostic but works with pointer-types, (i'll work on a fully type-agnostic one at a later date.)
  > Available through htils/hashmap.h
- Path manipulation.
  > Available through htils/path.h
- Type-agnostic dynamic arrays.
  > Available through htils/darray.h
- File IO.
  > Available through htils/file.h
- A tiny test suite.
  > Available through htils/test.h

> Final note in the features tab
>
> This library is mostly documented, so if you need help on how to use each respective api, read the header files in `./include`

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

- [ ] - Nix package.
- [ ] - More features.

## License

This project is licensed under the BSD-3 Clause License - see the [LICENSE](LICENSE) file for details.
