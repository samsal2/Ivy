#include "IvyGraphicsTemporaryBuffer.h"
#include "IvyGraphicsProgram.h"

static VkBufferUsageFlagBits ivyAsVulkanBufferUsage(uint32_t flags) {
  VkBufferUsageFlagBits bufferUsage = 0;

  if (IVY_VERTEX_BUFFER & flags)
    bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  if (IVY_INDEX_BUFFER & flags)
    bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  if (IVY_UNIFORM_BUFFER & flags)
    bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  if (IVY_SOURCE_BUFFER & flags)
    bufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  if (IVY_DESTINATION_BUFFER & flags)
    bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  return bufferUsage;
}

VkBuffer ivyCreateVulkanBuffer(VkDevice device, uint32_t flags,
    uint64_t size) {
  VkResult vulkanResult;
  VkBuffer buffer;
  VkBufferCreateInfo bufferCreateInfo;

  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.pNext = NULL;
  bufferCreateInfo.flags = 0;
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = ivyAsVulkanBufferUsage(flags);
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufferCreateInfo.queueFamilyIndexCount = 0;
  bufferCreateInfo.pQueueFamilyIndices = NULL;

  vulkanResult = vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return buffer;
}

IvyCode ivyCreateGraphicsTemporaryBufferProvider(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider) {
  IVY_UNUSED(context);
  IVY_UNUSED(graphicsAllocator);
  IVY_MEMSET(provider, 0, sizeof(*provider));
  return IVY_OK;
}

void ivyClearGraphicsTemporaryBufferProvider(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider) {
  int32_t i;
  for (i = 0; i < provider->garbageBufferCount; ++i) {
    if (provider->garbageBuffers[i]) {
      vkDestroyBuffer(context->device, provider->garbageBuffers[i], NULL);
      provider->garbageBuffers[i] = VK_NULL_HANDLE;
    }

    if (provider->garbageDescriptorSets[i]) {
      vkFreeDescriptorSets(context->device, context->globalDescriptorPool, 1,
          &provider->garbageDescriptorSets[i]);
      provider->garbageDescriptorSets[i] = VK_NULL_HANDLE;
    }

    ivyFreeGraphicsMemory(context, graphicsAllocator,
        &provider->garbageMemories[i]);
  }

  provider->currentBufferOffset = 0;
}
void ivyDestroyGraphicsTemporaryBufferProvider(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider) {
  ivyClearGraphicsTemporaryBufferProvider(context, graphicsAllocator,
      provider);

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

static uint64_t ivyAlignTo256(uint64_t value) {
  return ivyAlignTo(value, 256);
}

static uint64_t ivyNextGraphicsTemporaryBufferProviderSize(uint64_t size) {
  if (!size)
    return 4096;
  return ivyAlignTo256(size * 2);
}

static void ivyProvideGraphicsTemporaryBuffer(
    IvyGraphicsTemporaryBufferProvider *provider, uint64_t size,
    IvyGraphicsTemporaryBuffer *buffer) {
  uint64_t curretOffset = provider->currentBufferOffset;
  uint64_t nextOffset = ivyAlignTo256(curretOffset + size);

  buffer->data = ((uint8_t *)provider->currentMemory.data) + curretOffset;
  buffer->offset = provider->currentBufferOffset;
  buffer->buffer = provider->currentBuffer;
  buffer->descriptorSet = provider->currentDescriptorSet;

  provider->currentBufferOffset = nextOffset;
}

static IvyCode ivyMoveCurrentBufferToGarbageInGraphicsTemporaryBufferProvider(
    IvyGraphicsTemporaryBufferProvider *provider) {
  VkDescriptorSet currentDescriptorSet = provider->currentDescriptorSet;
  uint64_t const garbageBufferCount = provider->garbageBufferCount;
  uint64_t const nextGarbageBufferCount = garbageBufferCount + 1;

  if (nextGarbageBufferCount >= IVY_ARRAY_LENGTH(provider->garbageBuffers))
    return IVY_NO_GRAPHICS_MEMORY;

  provider->garbageBuffers[garbageBufferCount] = provider->currentBuffer;
  provider->garbageDescriptorSets[garbageBufferCount] = currentDescriptorSet;
  provider->garbageMemories[garbageBufferCount] = provider->currentMemory;
  return IVY_OK;
}

VkDescriptorSet ivyAllocateVulkanDescriptorSet(VkDevice device,
    VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout descriptorSetLayout) {
  VkResult vulkanResult;
  VkDescriptorSet descriptorSet;
  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;

  descriptorSetAllocateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorSetAllocateInfo.pNext = NULL;
  descriptorSetAllocateInfo.descriptorPool = descriptorPool;
  descriptorSetAllocateInfo.descriptorSetCount = 1;
  descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

  vulkanResult = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo,
      &descriptorSet);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return descriptorSet;
}

static void ivyWriteVulkanUniformDynamicDescriptorSet(VkDevice device,
    VkBuffer buffer, VkDescriptorSet descriptorSet, uint64_t uniformSize) {
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
  writeDescriptorSet.descriptorType =
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  writeDescriptorSet.pImageInfo = NULL;
  writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
  writeDescriptorSet.pTexelBufferView = NULL;

  vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

IvyCode ivyRequestGraphicsTemporaryBuffer(IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTemporaryBufferProvider *provider,
    VkDescriptorSetLayout uniformDescriptorSetLayout, uint64_t size,
    IvyGraphicsTemporaryBuffer *buffer) {
  if ((!provider->currentBuffer && !provider->currentMemory.memory) ||
      (provider->currentBufferOffset + size >= provider->currentBufferSize)) {
    IvyCode ivyCode = IVY_OK;
    VkBuffer newBuffer = VK_NULL_HANDLE;
    VkDescriptorSet newDescriptorSet = VK_NULL_HANDLE;
    IvyGraphicsMemory newMemory;
    uint64_t newSize = 0;

    newSize = ivyNextGraphicsTemporaryBufferProviderSize(
        provider->currentBufferSize);

    newBuffer = ivyCreateVulkanBuffer(context->device,
        IVY_VERTEX_BUFFER | IVY_INDEX_BUFFER | IVY_UNIFORM_BUFFER, newSize);
    if (!newBuffer)
      return IVY_NO_GRAPHICS_MEMORY;

    ivyCode = ivyAllocateAndBindGraphicsMemoryToBuffer(context,
        graphicsAllocator, IVY_HOST_VISIBLE, newBuffer, &newMemory);
    if (ivyCode) {
      vkDestroyBuffer(context->device, newBuffer, NULL);
      return IVY_NO_GRAPHICS_MEMORY;
    }

    newDescriptorSet = ivyAllocateVulkanDescriptorSet(context->device,
        context->globalDescriptorPool, uniformDescriptorSetLayout);
    if (!newDescriptorSet) {
      ivyFreeGraphicsMemory(context, graphicsAllocator, &newMemory);
      vkDestroyBuffer(context->device, newBuffer, NULL);
      return IVY_NO_GRAPHICS_MEMORY;
    }

    ivyWriteVulkanUniformDynamicDescriptorSet(context->device, newBuffer,
        newDescriptorSet, sizeof(IvyGraphicsProgramUniform));

    ivyCode = ivyMoveCurrentBufferToGarbageInGraphicsTemporaryBufferProvider(
        provider);
    if (ivyCode) {
      vkFreeDescriptorSets(context->device, context->globalDescriptorPool, 1,
          &newDescriptorSet);
      ivyFreeGraphicsMemory(context, graphicsAllocator, &newMemory);
      vkDestroyBuffer(context->device, newBuffer, NULL);
      return IVY_NO_GRAPHICS_MEMORY;
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
