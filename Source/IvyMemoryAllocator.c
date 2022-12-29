#include "IvyMemoryAllocator.h"

#include "IvyDeclarations.h"

#define IVY_MEMORY_ALLOCATOR_MAGIC 0xA50A6AAA

void ivySetupMemoryAllocatorBase(IvyMemoryAllocatorDispatch *dispatch,
    IvyMemoryAllocatorBase *base) {
  base->magic = IVY_MEMORY_ALLOCATOR_MAGIC;
  base->dispatch = dispatch;
}

void *ivyAllocateMemory(IvyAnyMemoryAllocator allocator, uint64_t size) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocate);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  return base->dispatch->allocate(allocator, size);
}

void *ivyAllocateAndZeroMemory(IvyAnyMemoryAllocator allocator, uint64_t count,
    uint64_t elementSize) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocateAndZero);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  return base->dispatch->allocateAndZero(allocator, count, elementSize);
}

void *ivyReallocateMemory(IvyAnyMemoryAllocator allocator, void *data,
    uint64_t newSize) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocateAndZero);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  return base->dispatch->reallocate(allocator, data, newSize);
}

void ivyFreeMemory(IvyAnyMemoryAllocator allocator, void *data) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->free);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  base->dispatch->free(allocator, data);
}

void ivyDestroyMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->destroy);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  base->dispatch->destroy(allocator);
}
