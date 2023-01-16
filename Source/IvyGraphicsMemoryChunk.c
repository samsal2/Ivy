#include "IvyGraphicsMemoryChunk.h"

#include "IvyRenderer.h"
#include "IvyVulkanUtilities.h"

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

IVY_INTERNAL uint32_t ivyFindVulkanMemoryTypeIndex(
    VkPhysicalDevice physicalDevice, uint32_t flags, uint32_t type) {
  uint32_t index;
  VkMemoryPropertyFlagBits memoryProperties;
  VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

  vkGetPhysicalDeviceMemoryProperties(physicalDevice,
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

IVY_INTERNAL VkResult ivyAllocateVulkanMemory(VkPhysicalDevice physicalDevice,
    VkDevice device, uint32_t flags, uint32_t type, uint64_t size,
    VkDeviceMemory *memory) {
  VkMemoryAllocateInfo memoryAllocateInfo;

  IVY_ASSERT(physicalDevice);
  IVY_ASSERT(device);

  memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocateInfo.pNext = NULL;
  memoryAllocateInfo.allocationSize = size;
  memoryAllocateInfo.memoryTypeIndex =
      ivyFindVulkanMemoryTypeIndex(physicalDevice, flags, type);
  IVY_ASSERT((uint32_t)-1 != memoryAllocateInfo.memoryTypeIndex);
  if ((uint32_t)-1 == memoryAllocateInfo.memoryTypeIndex) {
    return VK_ERROR_UNKNOWN;
  }

  return vkAllocateMemory(device, &memoryAllocateInfo, NULL, memory);
}

IVY_API void ivySetupEmptyGraphicsMemoryChunk(IvyGraphicsMemoryChunk *chunk) {
  chunk->data = NULL;
  chunk->flags = 0;
  chunk->type = 0;
  chunk->size = 0;
  chunk->owners = 0;
  chunk->memory = VK_NULL_HANDLE;
}

IVY_API IvyCode ivyAllocateGraphicsMemoryChunk(IvyGraphicsDevice *device,
    IvyGraphicsMemoryPropertyFlags flags, uint32_t type, uint64_t size,
    IvyGraphicsMemoryChunk *chunk) {
  VkResult vulkanResult;
  IvyCode ivyCode;

  chunk->flags = flags;
  chunk->type = type;
  chunk->size = size;
  chunk->owners = 1;

  vulkanResult = ivyAllocateVulkanMemory(device->physicalDevice,
      device->logicalDevice, flags, type, size, &chunk->memory);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  if (IVY_CPU_VISIBLE & flags) {
    vulkanResult = vkMapMemory(device->logicalDevice, chunk->memory, 0, size,
        0, &chunk->data);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }
  } else {
    chunk->data = NULL;
  }

  return IVY_OK;

error:
  ivyFreeGraphicsMemoryChunk(device, chunk);
  return ivyCode;
}

IVY_API void ivyFreeGraphicsMemoryChunk(IvyGraphicsDevice *device,
    IvyGraphicsMemoryChunk *chunk) {
  if (IVY_CPU_VISIBLE & chunk->flags) {
    vkUnmapMemory(device->logicalDevice, chunk->memory);
  }

  if (chunk->memory) {
    vkFreeMemory(device->logicalDevice, chunk->memory, NULL);
    chunk->memory = VK_NULL_HANDLE;
  }
}
