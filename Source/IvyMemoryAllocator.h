#ifndef IVY_MEMORY_ALLOCATOR_H
#define IVY_MEMORY_ALLOCATOR_H

#include "IvyDeclarations.h"

typedef void *IvyAnyMemoryAllocator;

typedef void *(*IvyAllocateMemoryCallback)(IvyAnyMemoryAllocator allocator,
    uint64_t size);

typedef void *(*IvyAllocateAndZeroMemoryCallback)(
    IvyAnyMemoryAllocator allocator, uint64_t count, uint64_t elementSize);

typedef void *(*IvyReallocateMemoryCallback)(IvyAnyMemoryAllocator allocator,
    void *data, uint64_t newSize);

typedef void (
    *IvyFreeMemoryCallback)(IvyAnyMemoryAllocator allocator, void *data);

typedef void (*IvyClearMemoryAllocatorCallback)(
    IvyAnyMemoryAllocator allocator);

typedef void (*IvyDestroyMemoryAllocator)(IvyAnyMemoryAllocator allocator);

typedef struct IvyMemoryAllocatorDispatch {
  IvyAllocateMemoryCallback allocate;
  IvyAllocateAndZeroMemoryCallback allocateAndZero;
  IvyReallocateMemoryCallback reallocate;
  IvyFreeMemoryCallback free;
  IvyClearMemoryAllocatorCallback clear;
  IvyDestroyMemoryAllocator destroy;
} IvyMemoryAllocatorDispatch;

typedef struct IvyMemoryAllocatorBase {
  uint64_t magic;
  IvyMemoryAllocatorDispatch const *dispatch;
} IvyMemoryAllocatorBase;

IVY_API void ivySetupMemoryAllocatorBase(
    IvyMemoryAllocatorDispatch const *dispatch, IvyMemoryAllocatorBase *base);

IVY_API void *ivyAllocateMemory(IvyAnyMemoryAllocator allocator,
    uint64_t size);

IVY_API void *ivyReallocateMemory(IvyAnyMemoryAllocator allocator, void *data,
    uint64_t newSize);

IVY_API void *ivyAllocateAndZeroMemory(IvyAnyMemoryAllocator allocator,
    uint64_t count, uint64_t elementSize);

IVY_API void ivyFreeMemory(IvyAnyMemoryAllocator allocator, void *data);

void ivyClearMemoryAllocator(IvyAnyMemoryAllocator allocator);

void ivyDestroyMemoryAllocator(IvyAnyMemoryAllocator allocator);

IVY_API IvyCode ivyGlobalDefaultMemoryAllocator(
    IvyAnyMemoryAllocator allocator);

IVY_API IvyAnyMemoryAllocator ivyGetGlobalMemoryAllocator(void);

IVY_API void ivyDestroyGlobalMemoryAllocator(void);
#endif
