#include "IvyMemoryAllocator.h"

#define IVY_MEMORY_ALLOCATOR_MAGIC 0xA50A6AAA

void ivySetupMemoryAllocatorBase(
    IvyMemoryAllocatorDispatch *dispatch,
    IvyMemoryAllocatorBase     *base) {
  base->magic    = IVY_MEMORY_ALLOCATOR_MAGIC;
  base->dispatch = dispatch;
}

void *ivyAllocateMemory(IvyAnyMemoryAllocator allocator, uint64_t size) {
  IvyMemoryAllocatorBase *base = allocator;
  return base->dispatch->allocate(allocator, size);
}

void *ivyAllocateAndZeroMemory(
    IvyAnyMemoryAllocator allocator,
    uint64_t              count,
    uint64_t              elementSize) {
  IvyMemoryAllocatorBase *base = allocator;
  return base->dispatch->allocateAndZero(allocator, count, elementSize);
}

void ivyFreeMemory(IvyAnyMemoryAllocator allocator, void *data) {
  IvyMemoryAllocatorBase *base = allocator;
  base->dispatch->free(allocator, data);
}

void ivyDestroyMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IvyMemoryAllocatorBase *base = allocator;
  base->dispatch->destroy(allocator);
}
