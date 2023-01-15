#ifndef IVY_GRAPHICS_MEMORY_CHUNK_H
#define IVY_GRAPHICS_MEMORY_CHUNK_H

#include "IvyDeclarations.h"

#include <vulkan/vulkan.h>

typedef struct IvyGraphicsDevice IvyGraphicsDevice;

typedef enum IvyGraphicsMemoryProperty {
  IVY_GRAPHICS_MEMORY_PROPERTY_GPU_LOCAL = 0x0001,
  IVY_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE = 0x0002,
} IvyGraphicsMemoryChunkProperty;
typedef uint64_t IvyGraphicsMemoryPropertyFlags;

typedef struct IvyGraphicsMemoryChunk {
  void *data;
  IvyGraphicsMemoryPropertyFlags flags;
  uint32_t type;
  uint64_t size;
  int32_t owners;
  VkDeviceMemory memory;
} IvyGraphicsMemoryChunk;

IVY_API void ivySetupEmptyGraphicsMemoryChunk(IvyGraphicsMemoryChunk *chunk);

IVY_API IvyCode ivyAllocateGraphicsMemoryChunk(IvyGraphicsDevice *device,
    IvyGraphicsMemoryPropertyFlags flags, uint32_t type, uint64_t size,
    IvyGraphicsMemoryChunk *chunk);

IVY_API void ivyFreeGraphicsMemoryChunk(IvyGraphicsDevice *device,
    IvyGraphicsMemoryChunk *chunk);

#endif
