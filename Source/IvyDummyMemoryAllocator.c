#include "IvyDummyMemoryAllocator.h"

#include <stdlib.h>

static void *ivyAllocateMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, uint64_t size) {
  IVY_UNUSED(allocator);
  return malloc(size);
}

static void *ivyAllocateAndZeroMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, uint64_t count, uint64_t elementSize) {
  IVY_UNUSED(allocator);
  return calloc(count, elementSize);
}

static void *ivyReallocateMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, void *data, uint64_t newSize) {
  IVY_UNUSED(allocator);
  return realloc(data, newSize);
}

static void ivyFreeMemoryFromDummyMemoryAllocator(
    IvyAnyMemoryAllocator allocator, void *data) {
  IVY_UNUSED(allocator);
  free(data);
}

static void ivyDestroyDummyMemoryAllocator(IvyAnyMemoryAllocator allocator) {
  IVY_UNUSED(allocator);
}

static IvyMemoryAllocatorDispatch dummyMemoryAllocatorDispatch = {
    ivyAllocateMemoryFromDummyMemoryAllocator,
    ivyAllocateAndZeroMemoryFromDummyMemoryAllocator,
    ivyReallocateMemoryFromDummyMemoryAllocator,
    ivyFreeMemoryFromDummyMemoryAllocator, ivyDestroyDummyMemoryAllocator};

IvyCode ivyCreateDummyMemoryAllocator(IvyDummyMemoryAllocator *allocator) {
  ivySetupMemoryAllocatorBase(&dummyMemoryAllocatorDispatch, &allocator->base);
  return IVY_OK;
}
