#ifndef IVY_ARENA_MEMORY_ALLOCATOR_H
#define IVY_ARENA_MEMORY_ALLOCATOR_H

#include "IvyMemoryAllocator.h"

typedef struct IvyArenaMemoryAllocator {
  IvyMemoryAllocatorBase base;
  uint64_t capacity;
  uint64_t position;
  uint64_t alignment;
  int32_t aliveAllocationCount;
  void *previousAllocation;
  void *data;
} IvyArenaMemoryAllocator;

IVY_API IvyCode ivyCreateArenaMemoryAllocator(uint64_t size,
    IvyArenaMemoryAllocator *allocator);

#endif
