/**
 * @note TODO: optionally add header-only support.
 */

#ifndef HTILS_H
#define HTILS_H

/** These headers wont be included if HTILS_IMEPLEMENTATION is defined. */
#ifndef HTILS_IMPLEMENTATION

/** Disables types headers for current translation unit if NO_TYPES are defined.
 */
#ifndef NO_TYPES
#include <htils/basictypes.h>
#endif

#ifdef USE_ALL
#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/darray.h>
#include <htils/file.h>
#include <htils/hashmap.h>
#include <htils/path.h>
#include <htils/string.h>
#include <htils/test.h>
#endif

#ifdef USE_MUSTHAVES
#include <htils/arena.h>
#include <htils/assert.h>
#include <htils/darray.h>
#include <htils/string.h>
#endif

#ifdef USE_ARENA
#include <htils/arena.h>
#endif

#ifdef USE_ASSERT
#include <htils/assert.h>
#endif

#ifdef USE_DARRAY
#include <htils/darray.h>
#endif

#ifdef USE_FILE
#include <htils/file.h>
#endif

#ifdef USE_HASHMAP
#include <htils/hashmap.h>
#endif

#ifdef USE_PATH
#include <htils/path.h>
#endif

#ifdef USE_STRING
#include <htils/string.h>
#endif

#ifdef USE_TEST
#include <htils/test.h>
#endif

#endif // !HTILS_IMPLEMENTATION

#endif // !HTILS_H
