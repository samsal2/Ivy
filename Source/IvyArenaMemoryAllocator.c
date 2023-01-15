#include "IvyArenaMemoryAllocator.h"

#include <stdlib.h>

IVY_INTERNAL uint64_t ivyRoundUp(uint64_t value, uint64_t by) {
  uint64_t const n = value / by;

  if (!value) {
    return 0;
  }

  return by * n + n;
}

#include <stdio.h>
IVY_INTERNAL void *ivyArenaMemoryAllocatorAllocate(
    IvyAnyMemoryAllocator allocator, uint64_t size) {
  uint64_t position;
  uint64_t newPosition;
  uint64_t requiredSize;
  IvyArenaMemoryAllocator *arenaAllocator = allocator;
  uint8_t *data = arenaAllocator->data;

  position = arenaAllocator->position;
  newPosition = position + ivyRoundUp(size, arenaAllocator->alignment);
  requiredSize = position + size;

  printf("%s\n", __PRETTY_FUNCTION__);
  printf("  arenaAllocator: %p, position: %llu\n  newPosition: %llu\n", allocator, position, newPosition);

  if (requiredSize > arenaAllocator->capacity)
    return NULL;

  arenaAllocator->position = newPosition;  

  ++arenaAllocator->aliveAllocationCount;
  return arenaAllocator->previousAllocation = data + position;
}

IVY_INTERNAL void *ivyArenaMemoryAllocatorAllocateAndZeroMemory(
    IvyAnyMemoryAllocator allocator, uint64_t count, uint64_t elementSize) {
  uint64_t size = count * elementSize;
  void *allocation = ivyAllocateMemory(allocator, size);
  IVY_MEMSET(allocation, 0, size);
  return allocation;
}

IVY_INTERNAL void *ivyArenaMemoryAllocatorReallocate(
    IvyAnyMemoryAllocator allocator, void *data, uint64_t newSize) {
  IVY_UNUSED(allocator);
  IVY_UNUSED(data);
  IVY_UNUSED(newSize);
  return NULL;
}

IVY_INTERNAL void ivyArenaMemoryAllocatorClear(
    IvyAnyMemoryAllocator allocator) {
  IvyArenaMemoryAllocator *arenaAllocator = allocator;
  arenaAllocator->position = 0; 
  arenaAllocator->previousAllocation = NULL; 
  arenaAllocator->aliveAllocationCount = 0; 
}

IVY_INTERNAL void ivyArenaMemoryAllocatorFree(IvyAnyMemoryAllocator allocator,
    void *data) {
  IvyArenaMemoryAllocator *arenaAllocator = allocator;

  IVY_UNUSED(data);

  IVY_ASSERT(arenaAllocator->aliveAllocationCount);

  if (arenaAllocator->aliveAllocationCount) {
    --arenaAllocator->aliveAllocationCount;
  } 

  if (!arenaAllocator->aliveAllocationCount) {
    ivyClearMemoryAllocator(allocator);
  }
}

IVY_INTERNAL void ivyArenaMemoryAllocatorDestroy(
    IvyAnyMemoryAllocator allocator) {
  IvyArenaMemoryAllocator *arenaAllocator = allocator;
  IVY_ASSERT(!arenaAllocator->aliveAllocationCount);
  free(arenaAllocator->data);
}

static IvyMemoryAllocatorDispatch arenaMemoryAllocatorDispatch = {
    ivyArenaMemoryAllocatorAllocate,
    ivyArenaMemoryAllocatorAllocateAndZeroMemory,
    ivyArenaMemoryAllocatorReallocate,
    ivyArenaMemoryAllocatorFree,
    ivyArenaMemoryAllocatorClear,
    ivyArenaMemoryAllocatorDestroy
};

IVY_API IvyCode ivyCreateArenaMemoryAllocator(uint64_t size,
    IvyArenaMemoryAllocator *allocator) {

  // FIXME(Samuel): instead of using malloc, give some way to overload this or
  // just ask for a buffer instead

  ivySetupMemoryAllocatorBase(&arenaMemoryAllocatorDispatch, &allocator->base);

  allocator->capacity = size;
  allocator->position = 0;
  allocator->alignment = sizeof(void *);
  allocator->aliveAllocationCount = 0;
  allocator->previousAllocation = NULL;
  allocator->data = malloc(allocator->capacity);
  if (!allocator->data) {
    return IVY_ERROR_NO_MEMORY;
  }

  return IVY_OK;
}
