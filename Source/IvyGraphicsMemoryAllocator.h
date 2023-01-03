#ifndef IVY_GRAPHICS_MEMORY_ALLOCATOR_H
#define IVY_GRAPHICS_MEMORY_ALLOCATOR_H

#include <vulkan/vulkan.h>

#include "IvyDeclarations.h"
#include "IvyGraphicsContext.h"

typedef void *IvyAnyGraphicsMemoryAllocator;

#define IVY_MAX_GRAPHICS_MEMORY_CHUNKS 64
#define IVY_GPU_LOCAL 0x0001
#define IVY_HOST_VISIBLE 0x0002

typedef struct IvyGraphicsMemory {
  void *data;
  int32_t slot;
  uint32_t flags;
  uint32_t type;
  uint64_t offset;
  VkDeviceMemory memory;
} IvyGraphicsMemory;

typedef IvyCode (*IvyAllocateGraphicsMemoryCallback)(
    IvyGraphicsContext *context, IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t flags, uint32_t type, uint64_t size, IvyGraphicsMemory *memory);

typedef void (*IvyFreeGraphicsMemoryCallback)(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator allocator, IvyGraphicsMemory *memory);

typedef void (*IvyReleaseGraphicsMemoryAllocatorCallback)(
    IvyGraphicsContext *context, IvyAnyGraphicsMemoryAllocator allocator);

typedef void (*IvyDestroyGraphicsMemoryAllocatorCallback)(
    IvyGraphicsContext *context, IvyAnyGraphicsMemoryAllocator allocator);

typedef struct IvyGraphicsMemoryAllocatorDispatch {
  IvyAllocateGraphicsMemoryCallback allocate;
  IvyFreeGraphicsMemoryCallback free;
  IvyReleaseGraphicsMemoryAllocatorCallback release;
  IvyDestroyGraphicsMemoryAllocatorCallback destroy;
} IvyGraphicsMemoryAllocatorDispatch;

typedef struct IvyGraphicsMemoryAllocatorBase {
  uint64_t magic;
  IvyGraphicsMemoryAllocatorDispatch const *dispatch;
} IvyGraphicsMemoryAllocatorBase;

IVY_API void ivySetupGraphicsMemoryAllocatorBase(
    IvyGraphicsMemoryAllocatorDispatch const *dispatch,
    IvyGraphicsMemoryAllocatorBase *base);

IVY_API void ivyDestroyGraphicsMemoryAllocator(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator allocator);

IVY_API IvyCode ivyAllocateGraphicsMemory(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator allocator, uint32_t flags, uint32_t type,
    uint64_t size, IvyGraphicsMemory *memory);

IVY_API void ivyFreeGraphicsMemory(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator allocator, IvyGraphicsMemory *memory);

IVY_API IvyCode ivyAllocateAndBindGraphicsMemoryToBuffer(
    IvyGraphicsContext *context, IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t flags, VkBuffer buffer, IvyGraphicsMemory *memory);

IVY_API IvyCode ivyAllocateAndBindGraphicsMemoryToImage(
    IvyGraphicsContext *context, IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t flags, VkImage image, IvyGraphicsMemory *allocation);

#endif
