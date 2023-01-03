#ifndef IVY_DUMMY_GRAPHICS_MEMORY_ALLOCATOR_H
#define IVY_DUMMY_GRAPHICS_MEMORY_ALLOCATOR_H

#include "IvyGraphicsContext.h"
#include "IvyGraphicsMemoryAllocator.h"
#include "IvyGraphicsMemoryChunk.h"

typedef struct IvyDummyGraphicsMemoryAllocator {
  IvyGraphicsMemoryAllocatorBase base;
  int32_t occupiedChunkCount;
  IvyGraphicsMemoryChunk chunks[64];
} IvyDummyGraphicsMemoryAllocator;

IVY_API IvyCode ivyCreateDummyGraphicsMemoryAllocator(
    IvyGraphicsContext *context, IvyDummyGraphicsMemoryAllocator *allocator);

#endif
