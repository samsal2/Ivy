#ifndef IVY_DECLARATIONS_H
#define IVY_DECLARATIONS_H

#include <stdint.h>

typedef int IvyCode;
typedef int IvyBool;

#define IVY_OK 0
#define IVY_UNKNOWN_ERROR -1
#define IVY_INVALID_VALUE -2
#define IVY_ALREADY_INITIALIZED -3
#define IVY_PLATAFORM_ERROR -4
#define IVY_NO_GRAPHICS_MEMORY -5

#define IVY_UNUSED(e) (void)e
#define IVY_ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#if 1
#include <assert.h>
#define IVY_ASSERT assert
#define IVY_UNREACHABLE() assert(0)
#define IVY_TODO() assert(0)
#endif

#define IVY_UNUSED(e) (void)e
#define IVY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define IVY_ENABLE_VULKAN_VALIDATION_LAYERS

#if 1
#include <string.h>
#define IVY_MEMCPY memcpy
#define IVY_MEMSET memset
#define IVY_STRNCMP strncmp
#endif

#if 1
#include <stdlib.h>
#define IVY_MALLOC malloc
#define IVY_FREE free
#define IVY_CALLOC calloc
#endif

#if 1
#include <stddef.h>
#define IVY_OFFSETOF offsetof
#endif

#if 1
#define IVY_STR(argument) #argument
#define IVY_LOG(format, ...) printf(IVY_STR(format) "\n  " format, __VA_ARGS__);
#endif

#endif
