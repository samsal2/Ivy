#include "IvyGraphicsVertexBuffer.h"

#include "IvyGraphicsDataUploader.h"
#include "IvyRenderer.h"
#include "IvyVulkanUtilities.h"

IVY_API IvyCode ivyCreateGraphicsVertexBuffer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, uint64_t size, void *data,
    IvyGraphicsVertexBuffer **buffer) {
  IvyCode ivyCode;
  VkResult vulkanResult;
  IvyGraphicsVertexBuffer *currentBuffer;

  currentBuffer = ivyAllocateMemory(allocator, size);
  IVY_ASSERT(currentBuffer);
  if (!currentBuffer) {
    ivyCode = IVY_ERROR_NO_MEMORY;
    goto error;
  }

  IVY_MEMSET(currentBuffer, 0, sizeof(*currentBuffer));

  vulkanResult = ivyCreateVulkanBuffer(renderer->device.logicalDevice,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      size, &currentBuffer->buffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyAllocateAndBindGraphicsMemoryToBuffer(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, IVY_GPU_LOCAL,
      currentBuffer->buffer, &currentBuffer->memory);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyUploadDataToVulkanBuffer(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->transientCommandPool, size, data, currentBuffer->buffer);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  return IVY_OK;

error:
  ivyDestroyGraphicsVertexBuffer(allocator, renderer, currentBuffer);
  *buffer = NULL;
  return ivyCode;
}

IVY_API void ivyDestroyGraphicsVertexBuffer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, IvyGraphicsVertexBuffer *buffer) {
  if (!buffer) {
    return;
  }

  ivyFreeGraphicsMemory(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, &buffer->memory);

  if (buffer->buffer) {
    vkDestroyBuffer(renderer->device.logicalDevice, buffer->buffer, NULL);
    buffer->buffer = VK_NULL_HANDLE;
  }

  ivyFreeMemory(allocator, buffer);
}
