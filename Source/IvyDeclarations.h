#ifndef IVY_DECLARATIONS_H
#define IVY_DECLARATIONS_H

#include <stdint.h>

typedef int IvyCode;
typedef int IvyBool;

#define IVY_INTERNAL static
#define IVY_LOCAL_PERSIST static

#define IVY_OK 0
#define IVY_ERROR_NO_DEVICE_FOUND -1
#define IVY_ERROR_NO_MEMORY -2
#define IVY_ERROR_INVALID_VALUE -3
#define IVY_ERROR_NO_GRAPHICS_MEMORY -4
#define IVY_ERROR_UNKNOWN -5
#define IVY_ERROR_INVALID_DYNAMIC_DISPATCH -6
#define IVY_ERROR_MORE_THAN_ONE_INSTANCE -7

#define IVY_UNUSED(e) (void)e
#define IVY_ARRAY_LENGTH(a) (long)(sizeof(a) / sizeof((a)[0]))

#if 1
#include <assert.h>
#define IVY_ASSERT assert
#define IVY_UNREACHABLE() assert(0)
#define IVY_TODO() assert(0)
#endif

#define IVY_UNUSED(e) (void)e
#define IVY_MAX(a, b) ((a) > (b) ? (a) : (b))

#if 1
#include <string.h>
#define IVY_MEMCPY memcpy
#define IVY_MEMSET memset
#define IVY_STRNCMP strncmp
#endif

#if 1
#include <stddef.h>
#define IVY_OFFSETOF offsetof
#endif

#endif
