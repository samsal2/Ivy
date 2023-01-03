#include "IvyGraphicsMemoryChunk.h"

IVY_INTERNAL VkMemoryPropertyFlagBits ivyGetVulkanMemoryProperties(
    uint32_t flags) {
  VkMemoryPropertyFlagBits properties = 0;

  if (IVY_GPU_LOCAL & flags) {
    properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  }

  if (IVY_CPU_VISIBLE & flags) {
    properties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }

  return properties;
}

IVY_INTERNAL uint32_t ivyFindVulkanMemoryTypeIndex(IvyGraphicsContext *context,
    uint32_t flags, uint32_t type) {
  uint32_t index;
  VkMemoryPropertyFlagBits memoryProperties;
  VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

  vkGetPhysicalDeviceMemoryProperties(context->physicalDevice,
      &physicalDeviceMemoryProperties);

  memoryProperties = ivyGetVulkanMemoryProperties(flags);

  index = 0;
  while (index < physicalDeviceMemoryProperties.memoryTypeCount) {
    VkPhysicalDeviceMemoryProperties *properties;
    VkMemoryType *memoryType;
    uint32_t propertyFlags;

    properties = &physicalDeviceMemoryProperties;
    memoryType = &properties->memoryTypes[index];
    propertyFlags = memoryType->propertyFlags;

    if ((propertyFlags & memoryProperties) && (type & (1U << index))) {
      return index;
    }

    ++index;
  }

  return (uint32_t)-1;
}

IVY_INTERNAL VkDeviceMemory ivyAllocateVulkanMemory(
    IvyGraphicsContext *context, uint32_t flags, uint32_t type,
    uint64_t size) {
  VkResult vulkanResult;
  VkDeviceMemory memory;
  VkMemoryAllocateInfo memoryAllocateInfo;

  IVY_ASSERT(context);

  memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocateInfo.pNext = NULL;
  memoryAllocateInfo.allocationSize = size;
  memoryAllocateInfo.memoryTypeIndex =
      ivyFindVulkanMemoryTypeIndex(context, flags, type);
  IVY_ASSERT((uint32_t)-1 != memoryAllocateInfo.memoryTypeIndex);
  if ((uint32_t)-1 == memoryAllocateInfo.memoryTypeIndex) {
    return VK_NULL_HANDLE;
  }

  vulkanResult =
      vkAllocateMemory(context->device, &memoryAllocateInfo, NULL, &memory);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    return VK_NULL_HANDLE;
  }

  return memory;
}

IVY_API void ivySetupEmptyGraphicsMemoryChunk(IvyGraphicsMemoryChunk *chunk) {
  chunk->data = NULL;
  chunk->flags = 0;
  chunk->type = 0;
  chunk->size = 0;
  chunk->owners = 0;
  chunk->memory = VK_NULL_HANDLE;
}

IVY_API IvyCode ivyAllocateGraphicsMemoryChunk(IvyGraphicsContext *context,
    uint32_t flags, uint32_t type, uint64_t size,
    IvyGraphicsMemoryChunk *chunk) {
  chunk->flags = flags;
  chunk->type = type;
  chunk->size = size;
  chunk->owners = 1;
  chunk->memory = ivyAllocateVulkanMemory(context, flags, type, size);
  if (!chunk->memory) {
    return IVY_NO_GRAPHICS_MEMORY;
  }

  if (IVY_CPU_VISIBLE & flags) {
    VkResult vulkanResult;
    vulkanResult =
        vkMapMemory(context->device, chunk->memory, 0, size, 0, &chunk->data);
    if (vulkanResult) {
      ivyFreeGraphicsMemoryChunk(context, chunk);
      return IVY_NO_GRAPHICS_MEMORY;
    }
  } else {
    chunk->data = NULL;
  }

  return IVY_OK;
}

IVY_API void ivyFreeGraphicsMemoryChunk(IvyGraphicsContext *context,
    IvyGraphicsMemoryChunk *chunk) {
  if (chunk->memory) {
    vkFreeMemory(context->device, chunk->memory, NULL);
    chunk->memory = VK_NULL_HANDLE;
  }
}
