#include "IvyDummyMemoryAllocator.h"

#include <stdlib.h>

static void *ivyAllocateMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, uint64_t size) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;
  ++dummyAllocator->aliveAllocationCount;
  return malloc(size);
}

static void *ivyAllocateAndZeroMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, uint64_t count, uint64_t elementSize) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;
  ++dummyAllocator->aliveAllocationCount;
  return calloc(count, elementSize);
}

static void *ivyReallocateMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, void *data, uint64_t newSize) {
  IVY_UNUSED(allocator);
  return realloc(data, newSize);
}

static void ivyFreeMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, void *data) {
  if (data) {
    IvyDummyMemoryAllocator *dummyAllocator = allocator;
    --dummyAllocator->aliveAllocationCount;
    free(data);
  }
}

#include <stdio.h>
static void ivyDestroyDummyMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IvyDummyMemoryAllocator *dummyAllocator = allocator;
  printf("%s\n %p->aliveAllocationCount: %i\n", __PRETTY_FUNCTION__, allocator,
      dummyAllocator->aliveAllocationCount);
  IVY_ASSERT(!dummyAllocator->aliveAllocationCount);
}

static IvyMemoryAllocatorDispatch dummyMemoryAllocatorDispatch = {
    ivyAllocateMemoryFromDummyMemoryAllocator,
    ivyAllocateAndZeroMemoryFromDummyMemoryAllocator,
    ivyReallocateMemoryFromDummyMemoryAllocator,
    ivyFreeMemoryFromDummyMemoryAllocator, ivyDestroyDummyMemoryAllocator};

IvyCode ivyCreateDummyMemoryAllocator(IvyDummyMemoryAllocator *allocator) {
  ivySetupMemoryAllocatorBase(&dummyMemoryAllocatorDispatch, &allocator->base);
  allocator->aliveAllocationCount = 0;
  return IVY_OK;
}
