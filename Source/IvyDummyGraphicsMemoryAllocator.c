#include "IvyDummyGraphicsMemoryAllocator.h"

#include "IvyGraphicsMemoryChunk.h"
#include "IvyRenderer.h"

IVY_INTERNAL IvyBool ivyIsGraphicsMemoryChunkEmpty(
    IvyGraphicsMemoryChunk *chunk) {
  IVY_ASSERT(chunk);
  return !chunk->memory;
}

IVY_INTERNAL IvyGraphicsMemoryChunk *
ivyDummyGraphicsMemoryAllocatorFindEmptyChunk(
    IvyDummyGraphicsMemoryAllocator *allocator) {
  int index;

  for (index = 0; index < IVY_ARRAY_LENGTH(allocator->chunks); ++index) {
    IvyGraphicsMemoryChunk *chunk = &allocator->chunks[index];

    if (ivyIsGraphicsMemoryChunkEmpty(chunk)) {
      return chunk;
    }
  }

  return NULL;
}

IVY_INTERNAL IvyCode ivyDummyGraphicsMemoryAllocatorAllocate(
    IvyGraphicsDevice *device, IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t flags, uint32_t type, uint64_t size, IvyGraphicsMemory *memory) {
  IvyCode ivyCode;
  IvyGraphicsMemoryChunk *chunk;
  IvyDummyGraphicsMemoryAllocator *dummyAllocator;

  IVY_ASSERT(device);
  IVY_ASSERT(allocator);
  IVY_ASSERT(memory);

  dummyAllocator = allocator;
  chunk = ivyDummyGraphicsMemoryAllocatorFindEmptyChunk(dummyAllocator);
  IVY_ASSERT(chunk);
  if (!chunk) {
    return IVY_ERROR_NO_MEMORY;
  }

  ivyCode = ivyAllocateGraphicsMemoryChunk(device, flags, type, size, chunk);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    return ivyCode;
  }

  ++dummyAllocator->occupiedChunkCount;

  memory->data = chunk->data;
  memory->slot = (int32_t)(chunk - &dummyAllocator->chunks[0]);
  memory->flags = chunk->flags;
  memory->type = chunk->type;
  memory->offset = 0;
  memory->memory = chunk->memory;

  return IVY_OK;
}

IVY_INTERNAL void ivyDummyGraphicsMemoryAllocatorFree(
    IvyGraphicsDevice *device, IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsMemory *allocation) {
  IvyGraphicsMemoryChunk *chunk;
  IvyDummyGraphicsMemoryAllocator *dummyAllocator;

  IVY_ASSERT(device);
  IVY_ASSERT(allocator);
  IVY_ASSERT(allocation);
  IVY_ASSERT(0 <= allocation->slot);

  dummyAllocator = allocator;
  --dummyAllocator->occupiedChunkCount;
  chunk = &dummyAllocator->chunks[allocation->slot];
  ivyFreeGraphicsMemoryChunk(device, chunk);
}

IVY_INTERNAL void ivyDestroyDummyGraphicsMemoryAllocator(
    IvyGraphicsDevice *device, IvyAnyGraphicsMemoryAllocator allocator) {
  int index;
  IvyDummyGraphicsMemoryAllocator *dummyAllocator;

  IVY_ASSERT(allocator);
  dummyAllocator = allocator;

  for (index = 0; index < IVY_ARRAY_LENGTH(dummyAllocator->chunks); ++index) {
    if (!ivyIsGraphicsMemoryChunkEmpty(&dummyAllocator->chunks[index])) {
      ivyFreeGraphicsMemoryChunk(device, &dummyAllocator->chunks[index]);
    }
  }
}

IVY_INTERNAL IvyGraphicsMemoryAllocatorDispatch const
    dummyGraphicsMemoryAllocatorDispatch = {
        ivyDummyGraphicsMemoryAllocatorAllocate,
        ivyDummyGraphicsMemoryAllocatorFree, NULL,
        ivyDestroyDummyGraphicsMemoryAllocator};

IVY_API IvyCode ivyCreateDummyGraphicsMemoryAllocator(
    IvyGraphicsDevice *device, IvyDummyGraphicsMemoryAllocator *allocator) {
  int index;

  IVY_ASSERT(device);
  IVY_ASSERT(allocator);

  IVY_MEMSET(allocator, 0, sizeof(*allocator));

  ivySetupGraphicsMemoryAllocatorBase(&dummyGraphicsMemoryAllocatorDispatch,
      &allocator->base);

  allocator->occupiedChunkCount = 0;
  for (index = 0; index < IVY_ARRAY_LENGTH(allocator->chunks); ++index) {
    ivySetupEmptyGraphicsMemoryChunk(&allocator->chunks[index]);
  }

  return IVY_OK;
}
