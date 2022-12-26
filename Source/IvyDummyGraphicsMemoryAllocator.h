#ifndef IVY_DUMMY_GRAPHICS_MEMORY_ALLOCATOR_H
#define IVY_DUMMY_GRAPHICS_MEMORY_ALLOCATOR_H

#include "IvyGraphicsContext.h"
#include "IvyGraphicsMemoryAllocator.h"
#include "IvyGraphicsMemoryChunk.h"

typedef struct IvyDummyGraphicsMemoryAllocator {
  IvyGraphicsMemoryAllocatorBase base;
  int32_t                        occupiedChunkCount;
  IvyGraphicsMemoryChunk         chunks[64];
} IvyDummyGraphicsMemoryAllocator;

IvyCode ivyCreateDummyGraphicsMemoryAllocator(
    IvyGraphicsContext              *context,
    IvyDummyGraphicsMemoryAllocator *allocator);

#endif
