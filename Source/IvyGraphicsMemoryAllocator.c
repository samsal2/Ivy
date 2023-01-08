#include "IvyGraphicsMemoryAllocator.h"

#include "IvyGraphicsContext.h"
#include "IvyVulkanUtilities.h"

#define IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC 0x5CA66E77

// TODO(samuel): static assert base is the first element of each allocator

IVY_API void ivySetupGraphicsMemoryAllocatorBase(
    IvyGraphicsMemoryAllocatorDispatch const *dispatch,
    IvyGraphicsMemoryAllocatorBase *base) {
  base->magic = IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC;
  base->dispatch = dispatch;
}

IVY_API void
ivyDestroyGraphicsMemoryAllocator(IvyGraphicsContext *context,
                                  IvyAnyGraphicsMemoryAllocator allocator) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->destroy);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  base->dispatch->destroy(context, allocator);
}

IVY_API IvyCode ivyAllocateGraphicsMemory(
    IvyGraphicsContext *context, IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t flags, uint32_t type, uint64_t size, IvyGraphicsMemory *memory) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocate);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  return base->dispatch->allocate(context, allocator, flags, type, size,
                                  memory);
}

IVY_API void ivyFreeGraphicsMemory(IvyGraphicsContext *context,
                                   IvyAnyGraphicsMemoryAllocator allocator,
                                   IvyGraphicsMemory *memory) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->free);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  base->dispatch->free(context, allocator, memory);
}

IVY_API IvyCode ivyAllocateAndBindGraphicsMemoryToBuffer(
    IvyGraphicsContext *context, IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t flags, VkBuffer buffer, IvyGraphicsMemory *memory) {
  int ivyCode;
  VkResult vulkanResult;
  VkMemoryRequirements memoryRequirements;

  IVY_ASSERT(context);
  IVY_ASSERT(allocator);
  IVY_ASSERT(buffer);
  IVY_ASSERT(memory);

  vkGetBufferMemoryRequirements(context->device, buffer, &memoryRequirements);

  ivyCode = ivyAllocateGraphicsMemory(context, allocator, flags,
                                      memoryRequirements.memoryTypeBits,
                                      memoryRequirements.size, memory);
  if (ivyCode) {
    return ivyCode;
  }

  vulkanResult = vkBindBufferMemory(context->device, buffer, memory->memory,
                                    memory->offset);
  if (vulkanResult) {
    ivyFreeGraphicsMemory(context, allocator, memory);
    return ivyVulkanResultAsIvyCode(vulkanResult);
  }

  return IVY_OK;
}

IVY_API IvyCode ivyAllocateAndBindGraphicsMemoryToImage(
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator, uint32_t flags,
    VkImage image, IvyGraphicsMemory *allocation) {
  IvyCode ivyCode;
  VkResult vulkanResult;
  VkMemoryRequirements memoryRequirements;

  IVY_ASSERT(context);
  IVY_ASSERT(graphicsAllocator);
  IVY_ASSERT(allocation);

  vkGetImageMemoryRequirements(context->device, image, &memoryRequirements);

  ivyCode = ivyAllocateGraphicsMemory(context, graphicsAllocator, flags,
                                      memoryRequirements.memoryTypeBits,
                                      memoryRequirements.size, allocation);
  if (ivyCode) {
    return ivyCode;
  }

  vulkanResult = vkBindImageMemory(context->device, image, allocation->memory,
                                   allocation->offset);
  if (vulkanResult) {
    ivyFreeGraphicsMemory(context, graphicsAllocator, allocation);
    return ivyVulkanResultAsIvyCode(vulkanResult);
  }

  return IVY_OK;
}
