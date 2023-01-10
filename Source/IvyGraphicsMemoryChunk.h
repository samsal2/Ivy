#ifndef IVY_GRAPHICS_MEMORY_CHUNK_H
#define IVY_GRAPHICS_MEMORY_CHUNK_H

#include "IvyDeclarations.h"

#include <vulkan/vulkan.h>

typedef struct IvyGraphicsDevice IvyGraphicsDevice;

#define IVY_GPU_LOCAL 0x0001
#define IVY_CPU_VISIBLE 0x0002

typedef struct IvyGraphicsMemoryChunk {
  void *data;
  uint32_t flags;
  uint32_t type;
  uint64_t size;
  int32_t owners;
  VkDeviceMemory memory;
} IvyGraphicsMemoryChunk;

IVY_API void ivySetupEmptyGraphicsMemoryChunk(IvyGraphicsMemoryChunk *chunk);

IVY_API IvyCode ivyAllocateGraphicsMemoryChunk(IvyGraphicsDevice *device,
    uint32_t flags, uint32_t type, uint64_t size,
    IvyGraphicsMemoryChunk *chunk);

IVY_API void ivyFreeGraphicsMemoryChunk(IvyGraphicsDevice *device,
    IvyGraphicsMemoryChunk *chunk);

#endif
