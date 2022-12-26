#ifndef IVY_GRAPHICS_MEMORY_CHUNK_H
#define IVY_GRAPHICS_MEMORY_CHUNK_H

#include "IvyDeclarations.h"
#include "IvyGraphicsContext.h"

#include <vulkan/vulkan.h>

struct ivy_graphics_context;

#define IVY_GPU_LOCAL 0x0001
#define IVY_CPU_VISIBLE 0x0002

typedef struct IvyGraphicsMemoryChunk {
  void          *data;
  uint32_t       flags;
  uint32_t       type;
  uint64_t       size;
  int32_t        owners;
  VkDeviceMemory memory;
} IvyGraphicsMemoryChunk;

void ivySetupEmptyGraphicsMemoryChunk(IvyGraphicsMemoryChunk *chunk);

IvyCode ivyAllocateGraphicsMemoryChunk(
    IvyGraphicsContext     *context,
    uint32_t                flags,
    uint32_t                type,
    uint64_t                size,
    IvyGraphicsMemoryChunk *chunk);

void ivyFreeGraphicsMemoryChunk(
    IvyGraphicsContext     *context,
    IvyGraphicsMemoryChunk *chunk);

#endif
