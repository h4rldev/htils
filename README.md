# htils & h2otils

A set of utilities I prefer for C for primarily Linux, but I'm pretty certain most if not all the utilities should work for Windows too.

## Table of Contents

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Features](#features)
  - [htils](#htils)
  - [h2otils](#h2otils)
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

- A small and flexible reserve and commit arena.

  > Available through htils/arena.h

- A string type based on unsigned 8bit integers, with tons of features for its manipulation.

  > Available through htils/string.h

- A custom assert that uses exit() instead of abort().

  > Available through htils/assert.h

- A simple stringmap, that isn't super type-agnostic, but it works well enough.

  > Available through htils/stringmap.h

- Path manipulation.

  > Available through htils/path.h

- Type-agnostic dynamic arrays.

  > Available through htils/darray.h

- File IO.

  > Available through htils/file.h

- An easy to use cli parser.

  > Available through htils/cli.h

- A dotenv parser.

  > Available through htils/dotenv.h

- A tiny test suite.
  > Available through htils/test.h

> Final note in the features tab
>
> This library is mostly documented, so if you need help on how to use each respective api, read the header files in `./include`
>
> If you want a nice little documentation site, run doxygen in a clone of this repo and open index.html in `./doc/html`
>
> Edit: The documentation site is now deployed to [https://htils.h4rl.dev](https://htils.h4rl.dev)!

### h2otils

- Cookie generation.

  > Available through h2otils/cookie.h

- String manipulation.

  > Available through h2otils/string.h

- More to come...

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
- [ ] - More features.

## License

This project is licensed under the BSD-3 Clause License - see the [LICENSE](LICENSE) file for details.
