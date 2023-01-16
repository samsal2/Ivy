
#include "IvyGraphicsDataUploader.h"

#include "IvyRenderer.h"
#include "IvyVulkanUtilities.h"

typedef struct IvyGraphicsUploadBuffer {
  VkBuffer buffer;
  IvyGraphicsMemory memory;
} IvyGraphicsUploadBuffer;

IVY_INTERNAL void ivyDestroyGraphicsUploadBuffer(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    IvyGraphicsUploadBuffer *buffer) {
  ivyFreeGraphicsMemory(device, graphicsMemoryAllocator, &buffer->memory);

  if (buffer->buffer) {
    vkDestroyBuffer(device->logicalDevice, buffer->buffer, NULL);
    buffer->buffer = VK_NULL_HANDLE;
  }
}

IVY_INTERNAL IvyCode ivyCreateGraphicsUploadBuffer(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator, uint64_t size,
    void *data, IvyGraphicsUploadBuffer *buffer) {
  VkResult vulkanResult;
  IvyCode ivyCode;

  buffer->memory.memory = VK_NULL_HANDLE;
  buffer->buffer = VK_NULL_HANDLE;

  vulkanResult = ivyCreateVulkanBuffer(device->logicalDevice,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, &buffer->buffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode =
      ivyAllocateAndBindGraphicsMemoryToBuffer(device, graphicsMemoryAllocator,
          IVY_CPU_VISIBLE, buffer->buffer, &buffer->memory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  IVY_MEMCPY(buffer->memory.data, data, size);

  return IVY_OK;

error:
  ivyDestroyGraphicsUploadBuffer(device, graphicsMemoryAllocator, buffer);
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
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    VkCommandPool commandPool, int32_t width, int32_t height,
    IvyPixelFormat format, void *data, VkImage image) {
  IvyCode ivyCode;
  VkResult vulkanResult;
  VkBufferImageCopy bufferImageCopy;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
  IvyGraphicsUploadBuffer uploadBuffer;

  uploadBuffer.buffer = VK_NULL_HANDLE;
  uploadBuffer.memory.memory = VK_NULL_HANDLE;

  vulkanResult = ivyAllocateAndBeginVulkanCommandBuffer(device->logicalDevice,
      commandPool, &commandBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyCreateGraphicsUploadBuffer(device, graphicsMemoryAllocator,
      width * height * ivyGetPixelFormatSize(format), data, &uploadBuffer);
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

  vkCmdCopyBufferToImage(commandBuffer, uploadBuffer.buffer, image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

  vulkanResult = ivyEndSubmitAndFreeVulkanCommandBuffer(device->logicalDevice,
      device->graphicsQueue, commandPool, commandBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyDestroyGraphicsUploadBuffer(device, graphicsMemoryAllocator,
      &uploadBuffer);

  return IVY_OK;

error:
  ivyDestroyGraphicsUploadBuffer(device, graphicsMemoryAllocator,
      &uploadBuffer);

  if (commandBuffer) {
    vkFreeCommandBuffers(device->logicalDevice, commandPool, 1,
        &commandBuffer);
  }

  return ivyCode;
}

IVY_API IvyCode ivyUploadDataToVulkanBuffer(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    VkCommandPool commandPool, uint64_t size, void *data, VkBuffer buffer) {
  IvyCode ivyCode = IVY_OK;
  VkResult vulkanResult;
  VkBufferCopy bufferCopy;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
  IvyGraphicsUploadBuffer uploadBuffer;

  uploadBuffer.buffer = VK_NULL_HANDLE;
  uploadBuffer.memory.memory = VK_NULL_HANDLE;

  vulkanResult = ivyAllocateAndBeginVulkanCommandBuffer(device->logicalDevice,
      commandPool, &commandBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyCreateGraphicsUploadBuffer(device, graphicsMemoryAllocator,
      size, data, &uploadBuffer);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  bufferCopy.srcOffset = 0;
  bufferCopy.dstOffset = 0;
  bufferCopy.size = size;

  vkCmdCopyBuffer(commandBuffer, uploadBuffer.buffer, buffer, 1, &bufferCopy);

  vulkanResult = ivyEndSubmitAndFreeVulkanCommandBuffer(device->logicalDevice,
      device->graphicsQueue, commandPool, commandBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyDestroyGraphicsUploadBuffer(device, graphicsMemoryAllocator,
      &uploadBuffer);

  return ivyCode;

error:
  ivyDestroyGraphicsUploadBuffer(device, graphicsMemoryAllocator,
      &uploadBuffer);

  if (commandBuffer) {
    vkFreeCommandBuffers(device->logicalDevice, commandPool, 1,
        &commandBuffer);
  }

  return ivyCode;
}
