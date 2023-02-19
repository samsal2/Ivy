#include "IvyMemoryAllocator.h"

#include "IvyDeclarations.h"
#include "IvyDummyMemoryAllocator.h"

#define IVY_MEMORY_ALLOCATOR_MAGIC 0xA50A6AAA

IVY_API void ivySetupMemoryAllocatorBase(
    IvyMemoryAllocatorDispatch const *dispatch, IvyMemoryAllocatorBase *base) {
  base->magic = IVY_MEMORY_ALLOCATOR_MAGIC;
  base->dispatch = dispatch;
}

IVY_API void *ivyAllocateMemory(IvyAnyMemoryAllocator allocator,
    uint64_t size) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocate);
  return base->dispatch->allocate(allocator, size);
}

IVY_API void *ivyAllocateAndZeroMemory(IvyAnyMemoryAllocator allocator,
    uint64_t count, uint64_t elementSize) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocateAndZero);
  return base->dispatch->allocateAndZero(allocator, count, elementSize);
}

IVY_API void *ivyReallocateMemory(IvyAnyMemoryAllocator allocator, void *data,
    uint64_t newSize) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->reallocate);
  return base->dispatch->reallocate(allocator, data, newSize);
}

IVY_API void ivyFreeMemory(IvyAnyMemoryAllocator allocator, void *data) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->free);
  base->dispatch->free(allocator, data);
}

IVY_API void ivyClearMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->clear);
  base->dispatch->clear(allocator);
}

IVY_API void ivyDestroyMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IvyMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->destroy);
  base->dispatch->destroy(allocator);
}

IVY_INTERNAL IvyBool createdDefaultAllocator = 0;
IVY_INTERNAL IvyAnyMemoryAllocator globalMemoryAllocator = NULL;

IVY_API IvyCode ivySetGlobalMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IvyMemoryAllocatorBase *base = allocator;

  if (IVY_MEMORY_ALLOCATOR_MAGIC != base->magic) {
    return IVY_ERROR_INVALID_DYNAMIC_DISPATCH;
  }

  globalMemoryAllocator = allocator;
  return IVY_OK;
}

IVY_API IvyAnyMemoryAllocator ivyGetGlobalMemoryAllocator(void) {
  IVY_LOCAL_PERSIST IvyDummyMemoryAllocator defaultMemoryAllocator;

  if (!globalMemoryAllocator && !createdDefaultAllocator) {
    ivyCreateDummyMemoryAllocator(&defaultMemoryAllocator);
    globalMemoryAllocator = &defaultMemoryAllocator;
    createdDefaultAllocator = 1;
  }

  return globalMemoryAllocator;
}

IVY_API void ivyDestroyGlobalMemoryAllocator(void) {
  ivyDestroyMemoryAllocator(ivyGetGlobalMemoryAllocator());
}
