#include "IvyGraphicsTemporaryBuffer.h"

#include "IvyGraphicsProgram.h"
#include "IvyVulkanUtilities.h"

IVY_INTERNAL VkBufferUsageFlagBits ivyAsVulkanBufferUsage(uint32_t flags) {
  VkBufferUsageFlagBits bufferUsage = 0;

  if (IVY_VERTEX_BUFFER & flags) {
    bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }

  if (IVY_INDEX_BUFFER & flags) {
    bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }

  if (IVY_UNIFORM_BUFFER & flags) {
    bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }

  if (IVY_SOURCE_BUFFER & flags) {
    bufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  if (IVY_DESTINATION_BUFFER & flags) {
    bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

  return bufferUsage;
}

IVY_API VkResult ivyCreateVulkanBuffer(VkDevice device, uint32_t flags,
                                       uint64_t size, VkBuffer *buffer) {
  VkBufferCreateInfo bufferCreateInfo;

  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.pNext = NULL;
  bufferCreateInfo.flags = 0;
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = ivyAsVulkanBufferUsage(flags);
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufferCreateInfo.queueFamilyIndexCount = 0;
  bufferCreateInfo.pQueueFamilyIndices = NULL;

  return vkCreateBuffer(device, &bufferCreateInfo, NULL, buffer);
}

IVY_API IvyCode ivyCreateGraphicsTemporaryBufferProvider(
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider) {
  IVY_UNUSED(context);
  IVY_UNUSED(graphicsAllocator);
  IVY_MEMSET(provider, 0, sizeof(*provider));
  return IVY_OK;
}

IVY_API void ivyClearGraphicsTemporaryBufferProvider(
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider) {
  int32_t index;
  for (index = 0; index < provider->garbageBufferCount; ++index) {
    if (provider->garbageBuffers[index]) {
      vkDestroyBuffer(context->device, provider->garbageBuffers[index], NULL);
      provider->garbageBuffers[index] = VK_NULL_HANDLE;
    }

    if (provider->garbageDescriptorSets[index]) {
      vkFreeDescriptorSets(context->device, context->globalDescriptorPool, 1,
                           &provider->garbageDescriptorSets[index]);
      provider->garbageDescriptorSets[index] = VK_NULL_HANDLE;
    }

    ivyFreeGraphicsMemory(context, graphicsAllocator,
                          &provider->garbageMemories[index]);
  }

  provider->currentBufferOffset = 0;
}

IVY_API void ivyDestroyGraphicsTemporaryBufferProvider(
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider) {
  ivyClearGraphicsTemporaryBufferProvider(context, graphicsAllocator, provider);

  if (provider->currentBuffer) {
    vkDestroyBuffer(context->device, provider->currentBuffer, NULL);
    provider->currentBuffer = VK_NULL_HANDLE;
  }

  if (provider->currentDescriptorSet) {
    vkFreeDescriptorSets(context->device, context->globalDescriptorPool, 1,
                         &provider->currentDescriptorSet);
    provider->currentDescriptorSet = VK_NULL_HANDLE;
  }

  ivyFreeGraphicsMemory(context, graphicsAllocator, &provider->currentMemory);
}

#define ivyAlignTo(value, to) (((value) + (to)-1) & ~((to)-1))

IVY_INTERNAL uint64_t ivyAlignTo256(uint64_t value) {
  return ivyAlignTo(value, 256);
}

#define IVY_DEFAULT_TEMPORARY_BUFFER_SIZE 4096

IVY_INTERNAL uint64_t
ivyNextGraphicsTemporaryBufferProviderSize(uint64_t size) {
  if (!size) {
    return IVY_DEFAULT_TEMPORARY_BUFFER_SIZE;
  }
  return ivyAlignTo256(size * 2);
}

IVY_INTERNAL void
ivyProvideGraphicsTemporaryBuffer(IvyGraphicsTemporaryBufferProvider *provider,
                                  uint64_t size,
                                  IvyGraphicsTemporaryBuffer *buffer) {
  uint64_t curretOffset = provider->currentBufferOffset;

  buffer->data = ((uint8_t *)provider->currentMemory.data) + curretOffset;
  buffer->size = size;
  buffer->offset = provider->currentBufferOffset;
  buffer->buffer = provider->currentBuffer;
  buffer->descriptorSet = provider->currentDescriptorSet;

  provider->currentBufferOffset = ivyAlignTo256(curretOffset + size);
}

IVY_INTERNAL IvyCode
ivyMoveCurrentBufferToGarbageInGraphicsTemporaryBufferProvider(
    IvyGraphicsTemporaryBufferProvider *provider) {
  VkDescriptorSet currentDescriptorSet = provider->currentDescriptorSet;
  uint64_t const garbageBufferCount = provider->garbageBufferCount;
  uint64_t const nextGarbageBufferCount = garbageBufferCount + 1;

  if (nextGarbageBufferCount < IVY_ARRAY_LENGTH(provider->garbageBuffers)) {
    provider->garbageBuffers[garbageBufferCount] = provider->currentBuffer;
    provider->garbageDescriptorSets[garbageBufferCount] = currentDescriptorSet;
    provider->garbageMemories[garbageBufferCount] = provider->currentMemory;
    return IVY_OK;
  }

  return IVY_ERROR_NO_GRAPHICS_MEMORY;
}

IVY_API VkResult ivyAllocateVulkanDescriptorSet(
    VkDevice device, VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet *descriptorSet) {
  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;

  descriptorSetAllocateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorSetAllocateInfo.pNext = NULL;
  descriptorSetAllocateInfo.descriptorPool = descriptorPool;
  descriptorSetAllocateInfo.descriptorSetCount = 1;
  descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

  return vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo,
                                  descriptorSet);
}

IVY_INTERNAL void
ivyWriteVulkanUniformDynamicDescriptorSet(VkDevice device, VkBuffer buffer,
                                          VkDescriptorSet descriptorSet,
                                          uint64_t uniformSize) {
  VkDescriptorBufferInfo descriptorBufferInfo;
  VkWriteDescriptorSet writeDescriptorSet;

  descriptorBufferInfo.buffer = buffer;
  descriptorBufferInfo.offset = 0;
  descriptorBufferInfo.range = uniformSize;

  writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSet.pNext = NULL;
  writeDescriptorSet.dstSet = descriptorSet;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.dstArrayElement = 0;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  writeDescriptorSet.pImageInfo = NULL;
  writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
  writeDescriptorSet.pTexelBufferView = NULL;

  vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

IVY_API IvyCode ivyRequestGraphicsTemporaryBuffer(
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider,
    VkDescriptorSetLayout uniformDescriptorSetLayout, uint64_t size,
    IvyGraphicsTemporaryBuffer *buffer) {
  if ((!provider->currentBuffer && !provider->currentMemory.memory) ||
      (provider->currentBufferOffset + size >= provider->currentBufferSize)) {
    VkResult vulkanResult;
    IvyCode ivyCode = IVY_OK;
    VkBuffer newBuffer = VK_NULL_HANDLE;
    VkDescriptorSet newDescriptorSet = VK_NULL_HANDLE;
    IvyGraphicsMemory newMemory;
    uint64_t newSize = 0;

    newSize =
        ivyNextGraphicsTemporaryBufferProviderSize(provider->currentBufferSize);

    vulkanResult = ivyCreateVulkanBuffer(context->device,
                                         IVY_VERTEX_BUFFER | IVY_INDEX_BUFFER |
                                             IVY_UNIFORM_BUFFER,
                                         newSize, &newBuffer);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      return ivyVulkanResultAsIvyCode(vulkanResult);
    }

    ivyCode = ivyAllocateAndBindGraphicsMemoryToBuffer(
        context, graphicsAllocator, IVY_HOST_VISIBLE, newBuffer, &newMemory);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      vkDestroyBuffer(context->device, newBuffer, NULL);
      return IVY_ERROR_NO_GRAPHICS_MEMORY;
    }

    vulkanResult = ivyAllocateVulkanDescriptorSet(
        context->device, context->globalDescriptorPool,
        uniformDescriptorSetLayout, &newDescriptorSet);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyFreeGraphicsMemory(context, graphicsAllocator, &newMemory);
      vkDestroyBuffer(context->device, newBuffer, NULL);
      return ivyVulkanResultAsIvyCode(vulkanResult);
    }

    ivyWriteVulkanUniformDynamicDescriptorSet(
        context->device, newBuffer, newDescriptorSet,
        sizeof(IvyGraphicsProgramUniform));

    ivyCode = ivyMoveCurrentBufferToGarbageInGraphicsTemporaryBufferProvider(
        provider);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      vkFreeDescriptorSets(context->device, context->globalDescriptorPool, 1,
                           &newDescriptorSet);
      ivyFreeGraphicsMemory(context, graphicsAllocator, &newMemory);
      vkDestroyBuffer(context->device, newBuffer, NULL);
      return ivyCode;
    }

    provider->currentDescriptorSet = newDescriptorSet;
    provider->currentBufferSize = newSize;
    provider->currentBufferOffset = 0;
    provider->currentBuffer = newBuffer;
    provider->currentDescriptorSet = newDescriptorSet;
    provider->currentMemory = newMemory;
  }

  ivyProvideGraphicsTemporaryBuffer(provider, size, buffer);

  return IVY_OK;
}
