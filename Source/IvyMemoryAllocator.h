#ifndef IVY_MEMORY_ALLOCATOR_H
#define IVY_MEMORY_ALLOCATOR_H

#include <stdint.h>

typedef void *IvyAnyMemoryAllocator;

typedef void *(
    *IvyAllocateMemoryCallback)(IvyAnyMemoryAllocator allocator, uint64_t size);

typedef void *(*IvyAllocateAndZeroMemoryCallback)(
    IvyAnyMemoryAllocator allocator,
    uint64_t              count,
    uint64_t              elementSize);

typedef void (
    *IvyFreeMemoryCallback)(IvyAnyMemoryAllocator allocator, void *data);

typedef void (*IvyDestroyMemoryAllocator)(IvyAnyMemoryAllocator allocator);

typedef struct IvyMemoryAllocatorDispatch {
  IvyAllocateMemoryCallback        allocate;
  IvyAllocateAndZeroMemoryCallback allocateAndZero;
  IvyFreeMemoryCallback            free;
  IvyDestroyMemoryAllocator        destroy;
} IvyMemoryAllocatorDispatch;

typedef struct IvyMemoryAllocatorBase {
  uint64_t                          magic;
  IvyMemoryAllocatorDispatch const *dispatch;
} IvyMemoryAllocatorBase;

void ivySetupMemoryAllocatorBase(
    IvyMemoryAllocatorDispatch *dispatch,
    IvyMemoryAllocatorBase     *base);

void *ivyAllocateMemory(IvyAnyMemoryAllocator allocator, uint64_t size);

void *ivyAllocateAndZeroMemory(
    IvyAnyMemoryAllocator allocator,
    uint64_t              count,
    uint64_t              elementSize);

void ivyFreeMemory(IvyAnyMemoryAllocator allocator, void *data);

void ivyDestroyMemoryAllocator(IvyAnyMemoryAllocator allocator);

#endif
