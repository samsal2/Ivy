#include "IvyVulkanUtilities.h"

IVY_API IvyCode ivyVulkanResultAsIvyCode(VkResult vulkanResult) {
  switch (vulkanResult) {
  case VK_SUCCESS:
    return IVY_OK;
  default:
    return -1;
  }
}

IVY_API VkResult ivyCreateVulkanImage(VkDevice device, int32_t width,
    int32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples,
    VkImageUsageFlags usage, VkFormat format, VkImage *image) {
  VkImageCreateInfo imageCreateInfo;

  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.pNext = NULL;
  imageCreateInfo.flags = 0;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = format;
  imageCreateInfo.extent.width = width;
  imageCreateInfo.extent.height = height;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = mipLevels;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = samples;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.usage = usage;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.queueFamilyIndexCount = 0;
  imageCreateInfo.pQueueFamilyIndices = NULL;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  return vkCreateImage(device, &imageCreateInfo, NULL, image);
}

IVY_API VkResult ivyCreateVulkanImageView(VkDevice device, VkImage image,
    VkImageAspectFlags aspect, VkFormat format, VkImageView *imageView) {
  VkImageViewCreateInfo imageViewCreateInfo;

  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.pNext = NULL;
  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.image = image;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = format;
  imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.subresourceRange.aspectMask = aspect;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount = 1;

  return vkCreateImageView(device, &imageViewCreateInfo, NULL, imageView);
}

IVY_API VkResult ivyCreateVulkanBuffer(VkDevice device,
    VkBufferUsageFlagBits flags, uint64_t size, VkBuffer *buffer) {
  VkBufferCreateInfo bufferCreateInfo;

  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.pNext = NULL;
  bufferCreateInfo.flags = 0;
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = flags;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufferCreateInfo.queueFamilyIndexCount = 0;
  bufferCreateInfo.pQueueFamilyIndices = NULL;

  return vkCreateBuffer(device, &bufferCreateInfo, NULL, buffer);
}

IVY_API VkResult ivyAllocateVulkanDescriptorSet(VkDevice device,
    VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet *descriptorSet) {
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

IVY_API VkResult ivyAllocateVulkanCommandBuffer(VkDevice device,
    VkCommandPool commandPool, VkCommandBuffer *commandBuffer) {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo;

  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.pNext = NULL;
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = 1;

  return vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
      commandBuffer);
}

IVY_API VkResult ivyAllocateAndBeginVulkanCommandBuffer(VkDevice device,
    VkCommandPool commandPool, VkCommandBuffer *commandBuffer) {
  VkResult vulkanResult;
  VkCommandBufferBeginInfo beginInfo;

  IVY_ASSERT(device);
  IVY_ASSERT(commandPool);
  IVY_ASSERT(commandBuffer);

  vulkanResult =
      ivyAllocateVulkanCommandBuffer(device, commandPool, commandBuffer);
  if (vulkanResult) {
    return vulkanResult;
  }

  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = NULL;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = NULL;

  vulkanResult = vkBeginCommandBuffer(*commandBuffer, &beginInfo);
  if (vulkanResult) {
    vkFreeCommandBuffers(device, commandPool, 1, commandBuffer);
    return vulkanResult;
  }

  return VK_SUCCESS;
}

IVY_API VkResult ivyEndSubmitAndFreeVulkanCommandBuffer(VkDevice device,
    VkQueue graphicsQueue, VkCommandPool commandPool,
    VkCommandBuffer commandBuffer) {
  VkResult vulkanResult;
  VkSubmitInfo submitInfo;

  IVY_ASSERT(device);
  IVY_ASSERT(commandPool);
  IVY_ASSERT(commandBuffer);

  vkEndCommandBuffer(commandBuffer);

  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = NULL;
  submitInfo.pWaitDstStageMask = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = NULL;

  vulkanResult = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  if (vulkanResult) {
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    return vulkanResult;
  }

  vulkanResult = vkQueueWaitIdle(graphicsQueue);
  if (vulkanResult) {
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    return vulkanResult;
  }

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

  return VK_SUCCESS;
}
