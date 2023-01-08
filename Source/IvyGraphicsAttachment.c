#include "IvyGraphicsAttachment.h"
#include "IvyGraphicsTexture.h"
#include "IvyVulkanUtilities.h"

IVY_INTERNAL VkImageUsageFlagBits
ivyAsVulkanImageUsage(IvyGraphicsAttachmentType type) {
  switch (type) {
  case IVY_COLOR_ATTACHMENT:
    return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  case IVY_DEPTH_ATTACHMENT:
    return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
}

IVY_INTERNAL VkImageAspectFlagBits
ivyAsVulkanImageAspect(IvyGraphicsAttachmentType type) {
  switch (type) {
  case IVY_COLOR_ATTACHMENT:
    return VK_IMAGE_ASPECT_COLOR_BIT;

  case IVY_DEPTH_ATTACHMENT:
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  }
}

IVY_INTERNAL VkFormat ivyGetGraphicsAttachmentFormat(
    IvyGraphicsContext *context, IvyGraphicsAttachmentType type) {
  switch (type) {
  case IVY_COLOR_ATTACHMENT:
    return context->surfaceFormat.format;

  case IVY_DEPTH_ATTACHMENT:
    return context->depthFormat;
  }
}

IVY_API IvyCode ivyCreateGraphicsAttachment(
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator, int32_t width,
    int32_t height, IvyGraphicsAttachmentType type,
    IvyGraphicsAttachment *attachment) {
  VkResult vulkanResult;
  IvyCode ivyCode = IVY_OK;

  IVY_MEMSET(attachment, 0, sizeof(*attachment));

  attachment->type = type;
  attachment->width = width;
  attachment->height = height;

  vulkanResult = ivyCreateVulkanImage(
      context->device, attachment->width, attachment->height, 1,
      context->attachmentSampleCounts, ivyAsVulkanImageUsage(attachment->type),
      ivyGetGraphicsAttachmentFormat(context, type), &attachment->image);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyAllocateAndBindGraphicsMemoryToImage(
      context, graphicsAllocator, IVY_GPU_LOCAL, attachment->image,
      &attachment->memory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  vulkanResult = ivyCreateVulkanImageView(
      context->device, attachment->image,
      ivyAsVulkanImageAspect(attachment->type),
      ivyGetGraphicsAttachmentFormat(context, type), &attachment->imageView);
  IVY_ASSERT(attachment->imageView);
  if (!attachment->imageView) {
    goto error;
  }

  return IVY_OK;

error:
  ivyDestroyGraphicsAttachment(context, graphicsAllocator, attachment);
  return ivyCode;
}

IVY_API void
ivyDestroyGraphicsAttachment(IvyGraphicsContext *context,
                             IvyAnyGraphicsMemoryAllocator allocator,
                             IvyGraphicsAttachment *attachment) {
  if (attachment->memory.memory) {
    ivyFreeGraphicsMemory(context, allocator, &attachment->memory);
    attachment->memory.memory = VK_NULL_HANDLE;
  }

  if (attachment->imageView) {
    vkDestroyImageView(context->device, attachment->imageView, NULL);
    attachment->imageView = VK_NULL_HANDLE;
  }

  if (attachment->image) {
    vkDestroyImage(context->device, attachment->image, NULL);
    attachment->image = VK_NULL_HANDLE;
  }
}
