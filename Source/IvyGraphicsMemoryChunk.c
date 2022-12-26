#include "IvyGraphicsMemoryChunk.h"

static VkMemoryPropertyFlagBits ivyGetVulkanMemoryProperties(uint32_t flags) {
  VkMemoryPropertyFlagBits properties = 0;

  if (IVY_GPU_LOCAL & flags)
    properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  if (IVY_CPU_VISIBLE & flags)
    properties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

  return properties;
}

static uint32_t ivyFindVulkanMemoryTypeIndex(
    IvyGraphicsContext *context,
    uint32_t            type,
    uint32_t            flags) {
  uint32_t                         index;
  VkMemoryPropertyFlagBits         memoryProperties;
  VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

  vkGetPhysicalDeviceMemoryProperties(
      context->physicalDevice,
      &physicalDeviceMemoryProperties);

  memoryProperties = ivyGetVulkanMemoryProperties(flags);

  while (index < physicalDeviceMemoryProperties.memoryTypeCount) {
    VkMemoryType *memoryType = &physicalDeviceMemoryProperties
                                    .memoryTypes[index];
    uint32_t propertyFlags = memoryType->propertyFlags;

    if ((propertyFlags & memoryProperties) && (type & (1U << type)))
      return index;

    ++index;
  }

  return (uint32_t)-1;
}

static VkDeviceMemory ivyAllocateVulkanMemory(
    IvyGraphicsContext *context,
    uint32_t            flags,
    uint32_t            type,
    uint64_t            size) {
  VkResult             vulkanResult;
  VkDeviceMemory       memory;
  VkMemoryAllocateInfo memoryAllocateInfo;

  IVY_ASSERT(context);

  memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocateInfo.pNext           = NULL;
  memoryAllocateInfo.allocationSize  = size;
  memoryAllocateInfo.memoryTypeIndex = ivyFindVulkanMemoryTypeIndex(
      context,
      type,
      flags);

  vulkanResult = vkAllocateMemory(
      context->device,
      &memoryAllocateInfo,
      NULL,
      &memory);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return memory;
}

void ivySetupEmptyGraphicsMemoryChunk(IvyGraphicsMemoryChunk *chunk) {
  chunk->data   = NULL;
  chunk->flags  = 0;
  chunk->type   = 0;
  chunk->size   = 0;
  chunk->owners = 0;
  chunk->memory = VK_NULL_HANDLE;
}

int ivyAllocateGraphicsMemoryChunk(
    IvyGraphicsContext     *context,
    uint32_t                flags,
    uint32_t                type,
    uint64_t                size,
    IvyGraphicsMemoryChunk *chunk) {
  chunk->flags  = flags;
  chunk->type   = type;
  chunk->size   = size;
  chunk->owners = 1;
  chunk->memory = ivyAllocateVulkanMemory(context, flags, type, size);
  if (!chunk->memory)
    return IVY_NO_GRAPHICS_MEMORY;

  if (IVY_CPU_VISIBLE & flags) {
    VkResult result;
    vkMapMemory(context->device, chunk->memory, 0, size, 0, &chunk->data);
    if (result) {
      ivyFreeGraphicsMemoryChunk(context, chunk);
      return IVY_NO_GRAPHICS_MEMORY;
    }
  } else {
    chunk->data = NULL;
  }

  return IVY_OK;
}

void ivyFreeGraphicsMemoryChunk(
    IvyGraphicsContext     *context,
    IvyGraphicsMemoryChunk *chunk) {
  vkFreeMemory(context->device, chunk->memory, NULL);
  ivySetupEmptyGraphicsMemoryChunk(chunk);
}
