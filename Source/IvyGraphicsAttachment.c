#include "IvyGraphicsAttachment.h"
#include "IvyGraphicsTexture.h"

static VkImageUsageFlagBits
ivyAsVulkanImageUsage(IvyGraphicsAttachmentType type) {
  switch (type) {
  case IVY_COLOR_ATTACHMENT:
    return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  case IVY_DEPTH_ATTACHMENT:
    return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
}

static VkImageAspectFlagBits
ivyAsVulkanImageAspect(IvyGraphicsAttachmentType type) {
  switch (type) {
  case IVY_COLOR_ATTACHMENT:
    return VK_IMAGE_ASPECT_COLOR_BIT;

  case IVY_DEPTH_ATTACHMENT:
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  }
}

static VkFormat ivyGetGraphicsAttachmentFormat(
    IvyGraphicsContext       *context,
    IvyGraphicsAttachmentType type) {
  switch (type) {
  case IVY_COLOR_ATTACHMENT:
    return context->surfaceFormat.format;

  case IVY_DEPTH_ATTACHMENT:
    return context->depthFormat;
  }
}

IvyCode ivyCreateGraphicsAttachment(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    IvyGraphicsAttachmentType     type,
    IvyGraphicsAttachment        *attachment) {
  IvyCode ivyCode;

  IVY_MEMSET(attachment, 0, sizeof(*attachment));

  attachment->type   = type;
  attachment->width  = width;
  attachment->height = height;

  attachment->image = ivyCreateVulkanImage(
      context->device,
      attachment->width,
      attachment->height,
      1,
      context->attachmentSampleCounts,
      ivyAsVulkanImageUsage(attachment->type),
      ivyGetGraphicsAttachmentFormat(context, type));
  IVY_ASSERT(attachment->image);
  if (!attachment->image)
    goto error;

  ivyCode = ivyAllocateAndBindGraphicsMemoryToImage(
      context,
      allocator,
      IVY_GPU_LOCAL,
      attachment->image,
      &attachment->memory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  attachment->imageView = ivyCreateVulkanImageView(
      context->device,
      attachment->image,
      ivyAsVulkanImageAspect(attachment->type),
      ivyGetGraphicsAttachmentFormat(context, type));
  IVY_ASSERT(attachment->imageView);
  if (!attachment->imageView)
    goto error;

  return IVY_OK;

error:
  ivyDestroyGraphicsAttachment(context, allocator, attachment);
  return IVY_NO_GRAPHICS_MEMORY;
}

void ivyDestroyGraphicsAttachment(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsAttachment        *attachment) {
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
