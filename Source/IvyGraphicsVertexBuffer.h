#ifndef IVY_GRAPHICS_VERTEX_BUFFER_H
#define IVY_GRAPHICS_VERTEX_BUFFER_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyMemoryAllocator.h"
#include "IvyVectorMath.h"

typedef struct IvyRenderer IvyRenderer;

typedef struct IvyGraphicsVertexBuffer {
  VkBuffer buffer;
  IvyGraphicsMemory memory;
} IvyGraphicsVertexBuffer;

IVY_API IvyCode ivyCreateGraphicsVertexBuffer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, uint64_t size, void *vertices,
    IvyGraphicsVertexBuffer **buffer);

IVY_API void ivyBindGraphicsVertexBuffer(IvyRenderer *renderer,
    IvyGraphicsVertexBuffer *buffer);

IVY_API void ivyDestroyGraphicsVertexBuffer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, IvyGraphicsVertexBuffer *buffer);

#endif
