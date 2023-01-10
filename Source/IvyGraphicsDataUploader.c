
#include "IvyGraphicsDataUploader.h"

#include "IvyRenderer.h"
#include "IvyVulkanUtilities.h"

IVY_INTERNAL IvyCode ivyCreateAndAllocateUploadBuffer(
    IvyGraphicsDevice *device, IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    uint64_t size, void *data, IvyGraphicsMemory *graphicsMemory,
    VkBuffer *uploadBuffer) {
  VkResult vulkanResult;
  IvyCode ivyCode;

  graphicsMemory->memory = VK_NULL_HANDLE;

  vulkanResult = ivyCreateVulkanBuffer(device->logicalDevice,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, uploadBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyAllocateAndBindGraphicsMemoryToBuffer(device, graphicsAllocator,
      IVY_HOST_VISIBLE, *uploadBuffer, graphicsMemory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  IVY_MEMCPY(graphicsMemory->data, data, size);

  return IVY_OK;

error:
  if (graphicsMemory->memory) {
    ivyFreeGraphicsMemory(device, graphicsAllocator, graphicsMemory);
    graphicsMemory->memory = VK_NULL_HANDLE;
  }

  if (*uploadBuffer) {
    vkDestroyBuffer(device->logicalDevice, *uploadBuffer, NULL);
    *uploadBuffer = NULL;
  }

  return ivyCode;
}

IVY_INTERNAL uint64_t ivyGetPixelFormatSize(IvyPixelFormat format) {
  switch (format) {
  case IVY_R8_UNORM:
    return 1 * sizeof(uint8_t);

  case IVY_RGBA8_SRGB:
    return 4 * sizeof(uint8_t);
  }
}

IVY_API IvyCode ivyUploadDataToVulkanImage(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator, VkCommandPool commandPool,
    int32_t width, int32_t height, IvyPixelFormat format, void *data,
    VkImage image) {
  IvyCode ivyCode;
  VkResult vulkanResult;
  VkBufferImageCopy bufferImageCopy;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
  VkBuffer uploadBuffer = VK_NULL_HANDLE;
  IvyGraphicsMemory uploadMemory;

  uploadMemory.memory = VK_NULL_HANDLE;

  vulkanResult = ivyAllocateAndBeginVulkanCommandBuffer(device->logicalDevice,
      commandPool, &commandBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyCreateAndAllocateUploadBuffer(device, graphicsAllocator,
      width * height * ivyGetPixelFormatSize(format), data, &uploadMemory,
      &uploadBuffer);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  bufferImageCopy.bufferOffset = 0;
  bufferImageCopy.bufferRowLength = 0;
  bufferImageCopy.bufferImageHeight = 0;
  bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  bufferImageCopy.imageSubresource.mipLevel = 0;
  bufferImageCopy.imageSubresource.baseArrayLayer = 0;
  bufferImageCopy.imageSubresource.layerCount = 1;
  bufferImageCopy.imageOffset.x = 0;
  bufferImageCopy.imageOffset.y = 0;
  bufferImageCopy.imageOffset.z = 0;
  bufferImageCopy.imageExtent.width = width;
  bufferImageCopy.imageExtent.height = height;
  bufferImageCopy.imageExtent.depth = 1;

  vkCmdCopyBufferToImage(commandBuffer, uploadBuffer, image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

  vulkanResult = ivyEndSubmitAndFreeVulkanCommandBuffer(device->logicalDevice,
      device->graphicsQueue, commandPool, commandBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyFreeGraphicsMemory(device, graphicsAllocator, &uploadMemory);
  vkDestroyBuffer(device->logicalDevice, uploadBuffer, NULL);

  return IVY_OK;

error:
  if (uploadMemory.memory) {
    ivyFreeGraphicsMemory(device, graphicsAllocator, &uploadMemory);
  }

  if (uploadBuffer) {
    vkDestroyBuffer(device->logicalDevice, uploadBuffer, NULL);
  }

  return ivyCode;
}
