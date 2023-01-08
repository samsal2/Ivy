#include "IvyArenaMemoryAllocator.h"

IVY_INTERNAL void *
ivyArenaMemoryAllocatorAllocate(IvyAnyMemoryAllocator allocator,
                                uint64_t size) {
  IvyArenaMemoryAllocator *dummyAllocator = allocator;
  ++dummyAllocator->aliveAllocationCount;
  IVY_DEBUG_LOG("dummyAllocator: %p\n  size: %lu\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
                allocator, size, dummyAllocator->aliveAllocationCount);
  return malloc(size);
}

IVY_INTERNAL void *ivyArenaMemoryAllocatorAllocateAndZeroMemory(
    IvyAnyMemoryAllocator allocator, uint64_t count, uint64_t elementSize) {
  IvyArenaMemoryAllocator *dummyAllocator = allocator;
  ++dummyAllocator->aliveAllocationCount;
  IVY_DEBUG_LOG("dummyAllocator: %p\n  count: %lu\n  elementSize: %lu\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
                allocator, count, elementSize,
                dummyAllocator->aliveAllocationCount);
  return calloc(count, elementSize);
}

IVY_INTERNAL void *
ivyArenaMemoryAllocatorReallocate(IvyAnyMemoryAllocator allocator, void *data,
                                  uint64_t newSize) {
  IvyArenaMemoryAllocator *dummyAllocator = allocator;
  IVY_UNUSED(allocator);
  IVY_UNUSED(dummyAllocator);
  IVY_DEBUG_LOG("dummyAllocator: %p\n  data: %p\n  newSize: %lu\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
                allocator, data, newSize, dummyAllocator->aliveAllocationCount);
  return realloc(data, newSize);
}

IVY_INTERNAL void
ivyArenaMemoryAllocatorClear(IvyAnyMemoryAllocator allocator) {
  IVY_UNUSED(allocator);
  IVY_TODO();
}

IVY_INTERNAL void ivyArenaMemoryAllocatorFree(IvyAnyMemoryAllocator allocator,
                                              void *data) {
  IvyArenaMemoryAllocator *dummyAllocator = allocator;

  if (data) {
    --dummyAllocator->aliveAllocationCount;
    free(data);
  }

  IVY_DEBUG_LOG("dummyAllocator: %p\n  data: %p\n  "
                "dummyAllocator->aliveAllocationCount: %i\n",
                allocator, data, dummyAllocator->aliveAllocationCount);
}

IVY_INTERNAL void
ivyDestroyArenaMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IvyArenaMemoryAllocator *dummyAllocator = allocator;
  IVY_UNUSED(allocator);
  IVY_UNUSED(dummyAllocator);
  IVY_DEBUG_LOG("dummyAllocator->aliveAllocationCount: %i\n",
                dummyAllocator->aliveAllocationCount);
  IVY_ASSERT(!dummyAllocator->aliveAllocationCount);
}

static IvyMemoryAllocatorDispatch arenaMemoryAllocatorDispatch = {
    NULL, NULL, NULL, NULL, NULL, NULL,
};

IVY_API IvyCode ivyCreateArenaMemoryAllocator(
    uint64_t size, IvyArenaMemoryAllocator *allocator) {

  ivySetupMemoryAllocatorBase(&arenaMemoryAllocatorDispatch, &allocator->base);

  return IVY_OK;
}
