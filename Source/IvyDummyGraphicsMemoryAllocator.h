#ifndef IVY_DUMMY_GRAPHICS_MEMORY_ALLOCATOR_H
#define IVY_DUMMY_GRAPHICS_MEMORY_ALLOCATOR_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyGraphicsMemoryChunk.h"

typedef struct IvyGraphicsDevice IvyGraphicsDevice;

typedef struct IvyDummyGraphicsMemoryAllocator {
  IvyGraphicsMemoryAllocatorBase base;
  int32_t occupiedChunkCount;
  IvyGraphicsMemoryChunk chunks[64];
} IvyDummyGraphicsMemoryAllocator;

IVY_API IvyCode ivyCreateDummyGraphicsMemoryAllocator(
    IvyGraphicsDevice *device, IvyDummyGraphicsMemoryAllocator *allocator);

#endif
