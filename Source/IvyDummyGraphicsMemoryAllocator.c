#include "IvyDummyGraphicsMemoryAllocator.h"
#include "IvyGraphicsMemoryChunk.h"

#include <stdio.h>

static IvyBool ivyIsGraphicsMemoryChunkEmpty(IvyGraphicsMemoryChunk *chunk) {
  IVY_ASSERT(chunk);
  return !chunk->memory;
}

static IvyGraphicsMemoryChunk *ivyFindEmptyGraphicsMemoryChunkInDummyAllocator(
    IvyDummyGraphicsMemoryAllocator *allocator) {
  int i;

  for (i = 0; i < IVY_ARRAY_LENGTH(allocator->chunks); ++i) {
    IvyGraphicsMemoryChunk *chunk = &allocator->chunks[i];

    if (ivyIsGraphicsMemoryChunkEmpty(chunk))
      return chunk;
  }

  return NULL;
}

static IvyCode ivyAllocateGraphicsMemoryFromDummyAllocator(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t                      flags,
    uint32_t                      type,
    uint64_t                      size,
    IvyGraphicsMemory            *memory) {
  IvyCode                          ivyCode;
  int32_t                          slot;
  IvyGraphicsMemoryChunk          *chunk;
  IvyDummyGraphicsMemoryAllocator *dummyAllocator;

  IVY_ASSERT(context);
  IVY_ASSERT(allocator);
  IVY_ASSERT(memory);

  dummyAllocator = allocator;
  chunk = ivyFindEmptyGraphicsMemoryChunkInDummyAllocator(dummyAllocator);
  IVY_ASSERT(chunk);
  if (!chunk)
    return IVY_NO_GRAPHICS_MEMORY;

  ivyCode = ivyAllocateGraphicsMemoryChunk(context, flags, type, size, chunk);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    return ivyCode;

  ++dummyAllocator->occupiedChunkCount;

  memory->data   = chunk->data;
  memory->slot   = (int32_t)(chunk - &dummyAllocator->chunks[0]);
  memory->flags  = chunk->flags;
  memory->type   = chunk->type;
  memory->offset = 0;
  memory->memory = chunk->memory;

  return IVY_OK;
}

static void ivyFreeGraphicsMemoryFromDummyAllocator(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsMemory            *allocation) {
  IvyGraphicsMemoryChunk          *chunk;
  IvyDummyGraphicsMemoryAllocator *dummyAllocator;

  IVY_ASSERT(context);
  IVY_ASSERT(allocator);
  IVY_ASSERT(allocation);
  IVY_ASSERT(0 <= allocation->slot);

  dummyAllocator = allocator;
  --dummyAllocator->occupiedChunkCount;
  chunk = &dummyAllocator->chunks[allocation->slot];
  ivyFreeGraphicsMemoryChunk(context, chunk);
}

static void ivyDestroyDummyGraphicsMemoryAllocator(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator) {
  int                              i;
  IvyDummyGraphicsMemoryAllocator *dummyAllocator;

  IVY_ASSERT(allocator);
  dummyAllocator = allocator;

  for (i = 0; i < IVY_ARRAY_LENGTH(dummyAllocator->chunks); ++i)
    if (!ivyIsGraphicsMemoryChunkEmpty(&dummyAllocator->chunks[i]))
      ivyFreeGraphicsMemoryChunk(context, &dummyAllocator->chunks[i]);
}

static IvyGraphicsMemoryAllocatorDispatch dummyGraphicsMemoryAllocatorDispatch =
    {ivyAllocateGraphicsMemoryFromDummyAllocator,
     ivyFreeGraphicsMemoryFromDummyAllocator,
     NULL,
     ivyDestroyDummyGraphicsMemoryAllocator};

IvyCode ivyCreateDummyGraphicsMemoryAllocator(
    IvyGraphicsContext              *context,
    IvyDummyGraphicsMemoryAllocator *allocator) {
  int i;

  if (!context || !allocator)
    return IVY_INVALID_VALUE;

  IVY_MEMSET(allocator, 0, sizeof(*allocator));

  ivySetupGraphicsMemoryAllocatorBase(
      &allocator->base,
      &dummyGraphicsMemoryAllocatorDispatch);

  allocator->occupiedChunkCount = 0;
  for (i = 0; i < IVY_ARRAY_LENGTH(allocator->chunks); ++i)
    ivySetupEmptyGraphicsMemoryChunk(&allocator->chunks[i]);

  return IVY_OK;
}
