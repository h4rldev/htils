#ifndef H2OTILS_H
#define H2OTILS_H

#ifndef H2OTILS_NO_TYPES
#include <htils/basictypes.h>
#endif // !H2OTILS_NO_TYPES

#ifdef H2OTILS_USE_ALL
#include <h2otils/cookie.h>
#include <h2otils/darray.h>
#include <h2otils/string.h>
#include <h2otils/stringmap.h>
#include <h2otils/test.h>
#endif // !H2OTILS_USE_ALL

#if defined(H2OTILS_USE_MUSTHAVES) || defined(H2OTILS_USE_STRING)
#include <h2otils/string.h>
#endif // !H2OTILS_USE_MUSTHAVES

#ifdef H2OTILS_USE_COOKIE
#include <h2otils/cookie.h>
#endif // !H2OTILS_USE_COOKIE

#ifdef H2OTILS_USE_DARRAY
#include <h2otils/darray.h>
#endif // !H2OTILS_USE_DARRAY

#ifdef H2OTILS_USE_STRINGMAP
#include <h2otils/stringmap.h>
#endif // !H2OTILS_USE_STRINGMAP

#ifdef H2OTILS_USE_TEST
#include <h2otils/test.h>
#endif // !H2OTILS_USE_TEST

#endif // !H2OTILS_H
