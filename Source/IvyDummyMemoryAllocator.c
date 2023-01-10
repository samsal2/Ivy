#include "IvyDummyMemoryAllocator.h"

#include "IvyLog.h"

#include <stdlib.h>

IVY_INTERNAL void *ivyDummyMemoryAllocatorAllocate(
    IvyAnyMemoryAllocator allocator, uint64_t size) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;
  ++dummyAllocator->aliveAllocationCount;
  IVY_DEBUG_LOG("dummyAllocator: %p\n  size: %lu\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
      allocator, size, dummyAllocator->aliveAllocationCount);
  return malloc(size);
}

IVY_INTERNAL void *ivyDummyMemoryAllocatorAllocateAndZeroMemory(
    IvyAnyMemoryAllocator allocator, uint64_t count, uint64_t elementSize) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;
  ++dummyAllocator->aliveAllocationCount;
  IVY_DEBUG_LOG("dummyAllocator: %p\n  count: %lu\n  elementSize: %lu\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
      allocator, count, elementSize, dummyAllocator->aliveAllocationCount);
  return calloc(count, elementSize);
}

IVY_INTERNAL void *ivyDummyMemoryAllocatorReallocate(
    IvyAnyMemoryAllocator allocator, void *data, uint64_t newSize) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;
  if (!data) {
    ++dummyAllocator->aliveAllocationCount;
  }
  IVY_UNUSED(allocator);
  IVY_UNUSED(dummyAllocator);
  IVY_DEBUG_LOG("dummyAllocator: %p\n  data: %p\n  newSize: %lu\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
      allocator, data, newSize, dummyAllocator->aliveAllocationCount);
  return realloc(data, newSize);
}

IVY_INTERNAL void ivyDummyMemoryAllocatorClear(
    IvyAnyMemoryAllocator allocator) {
  IVY_UNUSED(allocator);
  IVY_TODO();
}

IVY_INTERNAL void ivyDummyMemoryAllocatorFree(IvyAnyMemoryAllocator allocator,
    void *data) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;

  if (data) {
    --dummyAllocator->aliveAllocationCount;
    free(data);
  }

  IVY_DEBUG_LOG("dummyAllocator: %p\n  data: %p\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
      allocator, data, dummyAllocator->aliveAllocationCount);
}

IVY_INTERNAL void ivyDestroyDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;
  IVY_UNUSED(allocator);
  IVY_UNUSED(dummyAllocator);
  IVY_DEBUG_LOG("dummyAllocator->aliveAllocationCount: %i\n",
      dummyAllocator->aliveAllocationCount);
  IVY_ASSERT(!dummyAllocator->aliveAllocationCount);
}

IVY_INTERNAL IvyMemoryAllocatorDispatch const dummyMemoryAllocatorDispatch = {
    ivyDummyMemoryAllocatorAllocate,
    ivyDummyMemoryAllocatorAllocateAndZeroMemory,
    ivyDummyMemoryAllocatorReallocate, ivyDummyMemoryAllocatorFree,
    ivyDummyMemoryAllocatorClear, ivyDestroyDummyMemoryAllocator};

IVY_API IvyCode ivyCreateDummyMemoryAllocator(
    IvyDummyMemoryAllocator *allocator) {
  ivySetupMemoryAllocatorBase(&dummyMemoryAllocatorDispatch, &allocator->base);
  allocator->aliveAllocationCount = 0;
  return IVY_OK;
}
