#include "IvyGraphicsMemoryAllocator.h"

#include "IvyRenderer.h"
#include "IvyVulkanUtilities.h"

#define IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC 0x5CA66E77

// TODO(samuel): static assert base is the first element of each allocator

IVY_API void ivySetupGraphicsMemoryAllocatorBase(
    IvyGraphicsMemoryAllocatorDispatch const *dispatch,
    IvyGraphicsMemoryAllocatorBase *base) {
  base->magic = IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC;
  base->dispatch = dispatch;
}

IVY_API void ivyDestroyGraphicsMemoryAllocator(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator allocator) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->destroy);
  base->dispatch->destroy(device, allocator);
}

IVY_API IvyCode ivyAllocateGraphicsMemory(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator allocator, uint32_t flags, uint32_t type,
    uint64_t size, IvyGraphicsMemory *memory) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocate);
  return base->dispatch->allocate(device, allocator, flags, type, size,
      memory);
}

IVY_API void ivyFreeGraphicsMemory(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator allocator, IvyGraphicsMemory *memory) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->free);
  base->dispatch->free(device, allocator, memory);
}

IVY_API IvyCode ivyAllocateAndBindGraphicsMemoryToBuffer(
    IvyGraphicsDevice *device, IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t flags, VkBuffer buffer, IvyGraphicsMemory *memory) {
  int ivyCode;
  VkResult vulkanResult;
  VkMemoryRequirements memoryRequirements;

  IVY_ASSERT(device);
  IVY_ASSERT(allocator);
  IVY_ASSERT(buffer);
  IVY_ASSERT(memory);

  vkGetBufferMemoryRequirements(device->logicalDevice, buffer,
      &memoryRequirements);

  ivyCode = ivyAllocateGraphicsMemory(device, allocator, flags,
      memoryRequirements.memoryTypeBits, memoryRequirements.size, memory);
  if (ivyCode) {
    return ivyCode;
  }

  vulkanResult = vkBindBufferMemory(device->logicalDevice, buffer,
      memory->memory, memory->offset);
  if (vulkanResult) {
    ivyFreeGraphicsMemory(device, allocator, memory);
    return ivyVulkanResultAsIvyCode(vulkanResult);
  }

  return IVY_OK;
}

IVY_API IvyCode ivyAllocateAndBindGraphicsMemoryToImage(
    IvyGraphicsDevice *device, IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    uint32_t flags, VkImage image, IvyGraphicsMemory *allocation) {
  IvyCode ivyCode;
  VkResult vulkanResult;
  VkMemoryRequirements memoryRequirements;

  IVY_ASSERT(device);
  IVY_ASSERT(graphicsAllocator);
  IVY_ASSERT(allocation);

  vkGetImageMemoryRequirements(device->logicalDevice, image,
      &memoryRequirements);

  ivyCode = ivyAllocateGraphicsMemory(device, graphicsAllocator, flags,
      memoryRequirements.memoryTypeBits, memoryRequirements.size, allocation);
  if (ivyCode) {
    return ivyCode;
  }

  vulkanResult = vkBindImageMemory(device->logicalDevice, image,
      allocation->memory, allocation->offset);
  if (vulkanResult) {
    ivyFreeGraphicsMemory(device, graphicsAllocator, allocation);
    return ivyVulkanResultAsIvyCode(vulkanResult);
  }

  return IVY_OK;
}
