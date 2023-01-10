#include "IvyArenaMemoryAllocator.h"

#include <stdlib.h>

IVY_INTERNAL uint64_t ivyRoundUp(uint64_t value, uint64_t by) {
  uint64_t const n = value / by;

  if (!value) {
    return 0;
  }

  return by * n + n;
}

IVY_INTERNAL void *ivyArenaMemoryAllocatorAllocate(
    IvyAnyMemoryAllocator allocator, uint64_t size) {
  IvyArenaMemoryAllocator *arenaAllocator = allocator;
  uint64_t pastSize = arenaAllocator->size;
  uint64_t newSize = pastSize + ivyRoundUp(size, arenaAllocator->alignment);

  if (

  return arenaAllocator->previousAllocation = (void *)next;
}

IVY_INTERNAL void *ivyArenaMemoryAllocatorAllocateAndZeroMemory(
    IvyAnyMemoryAllocator allocator, uint64_t count, uint64_t elementSize) {
}

IVY_INTERNAL void *ivyArenaMemoryAllocatorReallocate(
    IvyAnyMemoryAllocator allocator, void *data, uint64_t newSize) {
}

IVY_INTERNAL void ivyArenaMemoryAllocatorClear(
    IvyAnyMemoryAllocator allocator) {
  IVY_UNUSED(allocator);
  IVY_TODO();
}

IVY_INTERNAL void ivyArenaMemoryAllocatorFree(IvyAnyMemoryAllocator allocator,
    void *data) {
}

IVY_INTERNAL void ivyDestroyArenaMemoryAllocator(
    IvyAnyMemoryAllocator allocator) {
}

static IvyMemoryAllocatorDispatch arenaMemoryAllocatorDispatch = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

IVY_API IvyCode ivyCreateArenaMemoryAllocator(uint64_t size,
    IvyArenaMemoryAllocator *allocator) {

  // FIXME(Samuel): instead of using malloc, give some way to overload this or
  // just ask for a buffer instead

  ivySetupMemoryAllocatorBase(&arenaMemoryAllocatorDispatch, &allocator->base);

  allocator->size = size;
  allocator->alignment = sizeof(void *);
  allocator->previousAllocation = NULL;
  allocator->data = malloc(allocator->size);
  if (!allocator->data) {
    return IVY_ERROR_NO_MEMORY;
  }

  return IVY_OK;
}
