
#include "IvyGraphicsDataUploader.h"

static VkBuffer ivyCreateVulkanUploadBuffer(VkDevice device, uint64_t size) {
  VkResult           vulkanResult;
  VkBuffer           buffer;
  VkBufferCreateInfo bufferCreateInfo;

  bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.pNext                 = NULL;
  bufferCreateInfo.flags                 = 0;
  bufferCreateInfo.size                  = size;
  bufferCreateInfo.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
  bufferCreateInfo.queueFamilyIndexCount = 0;
  bufferCreateInfo.pQueueFamilyIndices   = NULL;

  vulkanResult = vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return buffer;
}

static VkBuffer ivyCreateAndAllocateUploadBuffer(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    uint64_t                      size,
    void                         *data,
    IvyGraphicsMemory            *memory) {
  IvyCode  ivyCode;
  VkBuffer uploadBuffer = VK_NULL_HANDLE;

  memory->memory = VK_NULL_HANDLE;

  uploadBuffer = ivyCreateVulkanUploadBuffer(context->device, size);
  if (!uploadBuffer)
    goto error;

  ivyCode = ivyAllocateAndBindGraphicsMemoryToBuffer(
      context,
      allocator,
      IVY_HOST_VISIBLE,
      uploadBuffer,
      memory);
  if (ivyCode)
    goto error;

  IVY_MEMCPY(memory->data, data, size);

error:
  if (memory->memory) {
    ivyFreeGraphicsMemory(context, allocator, memory);
    memory->memory = VK_NULL_HANDLE;
  }

  if (uploadBuffer)
    vkDestroyBuffer(context->device, uploadBuffer, NULL);

  return VK_NULL_HANDLE;
}

static uint64_t ivyGetPixelFormatSize(IvyPixelFormat format) {
  switch (format) {
  case IVY_R8_UNORM:
    return 1 * sizeof(uint8_t);

  case IVY_RGBA8_SRGB:
    return 4 * sizeof(uint8_t);
  }
}

IvyCode ivyUploadDataToVulkanImage(
    IvyGraphicsContext            *context,
    IvyAnyGraphicsMemoryAllocator *allocator,
    int32_t                        width,
    int32_t                        height,
    IvyPixelFormat                 format,
    void                          *data,
    VkImage                        destImage) {
  IvyCode           ivyCode;
  VkBufferImageCopy bufferImageCopy;
  VkCommandBuffer   commandBuffer = VK_NULL_HANDLE;
  VkBuffer          uploadBuffer  = VK_NULL_HANDLE;
  IvyGraphicsMemory uploadMemory;

  uploadMemory.memory = VK_NULL_HANDLE;

  commandBuffer = ivyAllocateOneTimeCommandBuffer(context);
  if (!commandBuffer)
    goto error;

  uploadBuffer = ivyCreateAndAllocateUploadBuffer(
      context,
      allocator,
      width * height * ivyGetPixelFormatSize(format),
      data,
      &uploadMemory);
  if (!uploadBuffer)
    goto error;

  bufferImageCopy.bufferOffset                    = 0;
  bufferImageCopy.bufferRowLength                 = 0;
  bufferImageCopy.bufferImageHeight               = 0;
  bufferImageCopy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  bufferImageCopy.imageSubresource.mipLevel       = 0;
  bufferImageCopy.imageSubresource.baseArrayLayer = 0;
  bufferImageCopy.imageSubresource.layerCount     = 1;
  bufferImageCopy.imageOffset.x                   = 0;
  bufferImageCopy.imageOffset.y                   = 0;
  bufferImageCopy.imageOffset.z                   = 0;
  bufferImageCopy.imageExtent.width               = width;
  bufferImageCopy.imageExtent.height              = height;
  bufferImageCopy.imageExtent.depth               = 1;

  vkCmdCopyBufferToImage(
      commandBuffer,
      uploadBuffer,
      destImage,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &bufferImageCopy);

  ivyCode = ivySubmitOneTimeCommandBuffer(context, commandBuffer);
  if (ivyCode)
    goto error;

  ivyFreeGraphicsMemory(context, allocator, &uploadMemory);
  vkDestroyBuffer(context->device, uploadBuffer, NULL);
  ivyFreeOneTimeCommandBuffer(context, commandBuffer);

  return IVY_OK;

error:
  if (uploadMemory.memory)
    ivyFreeGraphicsMemory(context, allocator, &uploadMemory);

  if (uploadBuffer)
    vkDestroyBuffer(context->device, uploadBuffer, NULL);

  if (commandBuffer)
    ivyFreeOneTimeCommandBuffer(context, commandBuffer);

  return IVY_NO_GRAPHICS_MEMORY;
}
