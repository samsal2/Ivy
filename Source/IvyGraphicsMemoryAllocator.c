#include "IvyGraphicsMemoryAllocator.h"

#include "IvyGraphicsContext.h"

#define IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC 0x5CA66E77

// TODO(samuel): static assert base is the first element of each allocator

void ivySetupGraphicsMemoryAllocatorBase(
    IvyGraphicsMemoryAllocatorBase           *base,
    IvyGraphicsMemoryAllocatorDispatch const *dispatch) {
  base->magic    = IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC;
  base->dispatch = dispatch;
}

void ivyDestroyGraphicsMemoryAllocator(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->destroy);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  base->dispatch->destroy(context, allocator);
}

IvyCode ivyAllocateGraphicsMemory(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t                      flags,
    uint32_t                      type,
    uint64_t                      size,
    IvyGraphicsMemory            *memory) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->allocate);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  return base->dispatch
      ->allocate(context, allocator, flags, type, size, memory);
}
void ivyFreeGraphicsMemory(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsMemory            *memory) {
  IvyGraphicsMemoryAllocatorBase *base = allocator;
  IVY_ASSERT(base);
  IVY_ASSERT(base->dispatch);
  IVY_ASSERT(base->dispatch->free);
  IVY_ASSERT(IVY_GRAPHICS_MEMORY_ALLOCATOR_MAGIC == base->magic);
  base->dispatch->free(context, allocator, memory);
}

IvyCode ivyAllocateAndBindGraphicsMemoryToBuffer(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t                      flags,
    VkBuffer                      buffer,
    IvyGraphicsMemory            *memory) {
  int                  ivyCode;
  VkResult             vulkanResult;
  VkMemoryRequirements memoryRequirements;

  if (!allocator || !memory)
    return IVY_INVALID_VALUE;

  vkGetBufferMemoryRequirements(context->device, buffer, &memoryRequirements);

  ivyCode = ivyAllocateGraphicsMemory(
      context,
      allocator,
      flags,
      memoryRequirements.memoryTypeBits,
      memoryRequirements.size,
      memory);
  if (ivyCode)
    return ivyCode;

  vulkanResult = vkBindBufferMemory(
      context->device,
      buffer,
      memory->memory,
      memory->offset);
  if (vulkanResult) {
    ivyFreeGraphicsMemory(context, allocator, memory);
    return IVY_NO_GRAPHICS_MEMORY;
  }

  return IVY_OK;
}

int ivyAllocateAndBindGraphicsMemoryToImage(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    uint32_t                      flags,
    VkImage                       image,
    IvyGraphicsMemory            *allocation) {
  int                  ivyCode;
  VkResult             vulkanResult;
  VkMemoryRequirements memoryRequirements;

  if (!allocator || !allocation)
    return IVY_INVALID_VALUE;

  vkGetImageMemoryRequirements(context->device, image, &memoryRequirements);

  ivyCode = ivyAllocateGraphicsMemory(
      context,
      allocator,
      flags,
      memoryRequirements.memoryTypeBits,
      memoryRequirements.size,
      allocation);
  if (ivyCode)
    return ivyCode;

  vulkanResult = vkBindImageMemory(
      context->device,
      image,
      allocation->memory,
      allocation->offset);
  if (vulkanResult) {
    ivyFreeGraphicsMemory(context, allocator, allocation);
    return IVY_NO_GRAPHICS_MEMORY;
  }

  return IVY_OK;
}
