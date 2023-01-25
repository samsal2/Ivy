#ifndef IVY_GRAPHICS_INDEX_BUFFER_H
#define IVY_GRAPHICS_INDEX_BUFFER_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyMemoryAllocator.h"
#include "IvyVectorMath.h"

typedef struct IvyRenderer IvyRenderer;

typedef uint32_t IvyGraphicsIndex;

typedef struct IvyGraphicsIndexBuffer {
  VkBuffer buffer;
  IvyGraphicsMemory memory;
} IvyGraphicsIndexBuffer;

IVY_API IvyCode ivyCreateGraphicsIndexBuffer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, uint64_t size, void *indices,
    IvyGraphicsIndexBuffer **buffer);

IVY_API void ivyBindGraphicsIndexBuffer(IvyRenderer *renderer,
    IvyGraphicsIndexBuffer *buffer);

IVY_API void ivyDestroyGraphicsIndexBuffer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, IvyGraphicsIndexBuffer *buffer);

#endif
