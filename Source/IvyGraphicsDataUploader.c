
#include "IvyGraphicsDataUploader.h"

#include "IvyGraphicsTemporaryBuffer.h"

static VkBuffer ivyCreateAndAllocateUploadBuffer(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    uint64_t                      size,
    void                         *data,
    IvyGraphicsMemory            *memory) {
  IvyCode  ivyCode;
  VkBuffer uploadBuffer = VK_NULL_HANDLE;

  memory->memory = VK_NULL_HANDLE;

  uploadBuffer = ivyCreateVulkanBuffer(
      context->device,
      IVY_SOURCE_BUFFER,
      size);
  IVY_ASSERT(uploadBuffer);
  if (!uploadBuffer)
    goto error;

  ivyCode = ivyAllocateAndBindGraphicsMemoryToBuffer(
      context,
      allocator,
      IVY_HOST_VISIBLE,
      uploadBuffer,
      memory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  IVY_MEMCPY(memory->data, data, size);

  return uploadBuffer;

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
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    IvyPixelFormat                format,
    void                         *data,
    VkImage                       destImage) {
  IvyCode           ivyCode;
  VkBufferImageCopy bufferImageCopy;
  VkCommandBuffer   commandBuffer = VK_NULL_HANDLE;
  VkBuffer          uploadBuffer  = VK_NULL_HANDLE;
  IvyGraphicsMemory uploadMemory;

  uploadMemory.memory = VK_NULL_HANDLE;

  commandBuffer = ivyAllocateOneTimeCommandBuffer(context);
  IVY_ASSERT(commandBuffer);
  if (!commandBuffer)
    goto error;

  uploadBuffer = ivyCreateAndAllocateUploadBuffer(
      context,
      allocator,
      width * height * ivyGetPixelFormatSize(format),
      data,
      &uploadMemory);
  IVY_ASSERT(uploadBuffer);
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
  IVY_ASSERT(!ivyCode);
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
