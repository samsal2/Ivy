#ifndef IVY_DUMMY_MEMORY_ALLOCATOR_H
#define IVY_DUMMY_MEMORY_ALLOCATOR_H

#include "IvyDeclarations.h"
#include "IvyMemoryAllocator.h"

typedef struct IvyDummyMemoryAllocator {
  IvyMemoryAllocatorBase base;
  int32_t aliveAllocationCount;
} IvyDummyMemoryAllocator;

IvyCode ivyCreateDummyMemoryAllocator(IvyDummyMemoryAllocator *allocator);

#endif
