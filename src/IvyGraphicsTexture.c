#include "IvyGraphicsTexture.h"
#include "IvyGraphicsDataUploader.h"

#ifndef IVY_FLOOR
#include <math.h>
#define IVY_FLOOR floor
#define IVY_LOG2 log2
#endif

static VkFormat ivyAsVulkanFormat(IvyPixelFormat format) {
  switch (format) {
  case IVY_R8_UNORM:
    return VK_FORMAT_R8_UNORM;

  case IVY_RGBA8_SRGB:
    return VK_FORMAT_R8G8B8A8_SRGB;
  }
}

static IvyPixelFormat ivyAsPixelFormat(VkFormat format) {
  switch (format) {
  case VK_FORMAT_R8_UNORM:
    return IVY_R8_UNORM;

  case VK_FORMAT_R8G8B8A8_SRGB:
    return IVY_RGBA8_SRGB;
  }
}

static uint64_t ivyGetPixelFormatSize(IvyPixelFormat format) {
  switch (format) {
  case IVY_R8_UNORM:
    return 1 * sizeof(uint8_t);

  case IVY_RGBA8_SRGB:
    return 4 * sizeof(uint8_t);
  }
}

static uint32_t ivyCalculateMipLevels(int32_t width, int32_t height) {
  return (uint32_t)(IVY_FLOOR(IVY_LOG2(IVY_MAX(width, height))) + 1);
}

VkImage ivyCreateVulkanImage(
    VkDevice              device,
    int32_t               width,
    int32_t               height,
    uint32_t              mipLevels,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags     usage,
    VkFormat              format) {
  VkResult          vulkanResult;
  VkImage           image;
  VkImageCreateInfo imageCreateInfo;

  if (!device)
    return VK_NULL_HANDLE;

  imageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.pNext                 = NULL;
  imageCreateInfo.flags                 = 0;
  imageCreateInfo.imageType             = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format                = format;
  imageCreateInfo.extent.width          = width;
  imageCreateInfo.extent.height         = height;
  imageCreateInfo.extent.depth          = 1;
  imageCreateInfo.mipLevels             = mipLevels;
  imageCreateInfo.arrayLayers           = 1;
  imageCreateInfo.samples               = samples;
  imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.usage                 = usage;
  imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.queueFamilyIndexCount = 0;
  imageCreateInfo.pQueueFamilyIndices   = NULL;
  imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

  vulkanResult = vkCreateImage(device, &imageCreateInfo, NULL, &image);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return image;
}

VkImageView ivyCreateVulkanImageView(
    VkDevice           device,
    VkImage            image,
    VkImageAspectFlags aspect,
    VkFormat           format) {
  VkResult              vulkanResult;
  VkImageView           imageView;
  VkImageViewCreateInfo imageViewCreateInfo;

  imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.pNext        = NULL;
  imageViewCreateInfo.flags        = 0;
  imageViewCreateInfo.image        = image;
  imageViewCreateInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format       = format;
  imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.subresourceRange.aspectMask     = aspect;
  imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
  imageViewCreateInfo.subresourceRange.levelCount     = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount     = 1;

  vulkanResult = vkCreateImageView(
      device,
      &imageViewCreateInfo,
      NULL,
      &imageView);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return imageView;
}

IvyCode ivyChangeVulkanImageLayout(
    IvyGraphicsContext *context,
    uint32_t            mipLevels,
    VkImage             image,
    VkImageLayout       sourceLayout,
    VkImageLayout       destinationLayout) {
  IvyCode ivyCode;

  VkImageMemoryBarrier imageMemoryBarrier;
  VkCommandBuffer      commandBuffer;

  VkPipelineStageFlags sourceStage      = VK_PIPELINE_STAGE_NONE_KHR;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE_KHR;

  commandBuffer = ivyAllocateOneTimeCommandBuffer(context);
  if (!commandBuffer)
    return IVY_NO_GRAPHICS_MEMORY;

  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.pNext = NULL;
  /* image_memory_imageMemoryBarrier.srcAccessMask = Defined later */
  /* image_memory_imageMemoryBarrier.dstAccessMask = Defined later */
  imageMemoryBarrier.oldLayout                       = sourceStage;
  imageMemoryBarrier.newLayout                       = destinationLayout;
  imageMemoryBarrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image                           = image;
  imageMemoryBarrier.subresourceRange.baseMipLevel   = 0;
  imageMemoryBarrier.subresourceRange.levelCount     = mipLevels;
  imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
  imageMemoryBarrier.subresourceRange.layerCount     = 1;

  if (VK_IMAGE_LAYOUT_UNDEFINED == sourceLayout &&
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == sourceLayout &&
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (
      VK_IMAGE_LAYOUT_UNDEFINED == sourceLayout &&
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
    imageMemoryBarrier
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == sourceLayout &&
      VK_IMAGE_LAYOUT_GENERAL == destinationLayout) {
    /* FIXME(samuel): is this correct? */
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  } else {
    IVY_UNREACHABLE();
  }

  if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  } else if (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  } else if (VK_IMAGE_LAYOUT_GENERAL == destinationLayout) {
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  } else {
    IVY_UNREACHABLE();
  }

  vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage,
      destinationStage,
      0,
      0,
      NULL,
      0,
      NULL,
      1,
      &imageMemoryBarrier);

  // NOTE: even if it fails we are going to free the commandBuffer
  ivyCode = ivySubmitOneTimeCommandBuffer(context, commandBuffer);
  ivyFreeOneTimeCommandBuffer(context, commandBuffer);
  return ivyCode;
}

IvyCode ivyGenerateVulkanImageMips(
    IvyGraphicsContext *context,
    int32_t             width,
    int32_t             height,
    uint32_t            mipLevels,
    VkImage             image) {
  uint32_t             i;
  IvyCode              ivyCode;
  VkImageMemoryBarrier imageMemoryBarrier;
  VkCommandBuffer      commandBuffer;

  if (1 == mipLevels || 0 == mipLevels)
    return IVY_OK;

  commandBuffer = ivyAllocateOneTimeCommandBuffer(context);
  if (!commandBuffer)
    return IVY_NO_GRAPHICS_MEMORY;

  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.pNext = NULL;
  /* image_memory_imageMemoryBarrier.srcAccessMask =  later */
  /* image_memory_imageMemoryBarrier.dstAccessMask =  later */
  /* image_memory_imageMemoryBarrier.oldLayout =  later */
  /* image_memory_imageMemoryBarrier.newLayout =  later */
  imageMemoryBarrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image                       = image;
  imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
  imageMemoryBarrier.subresourceRange.layerCount     = 1;
  imageMemoryBarrier.subresourceRange.levelCount     = 1;

  for (i = 0; i < (mipLevels - 1); ++i) {
    VkImageBlit imageBlit;

    imageMemoryBarrier.subresourceRange.baseMipLevel = i;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &imageMemoryBarrier);

    imageBlit.srcOffsets[0].x               = 0;
    imageBlit.srcOffsets[0].y               = 0;
    imageBlit.srcOffsets[0].z               = 0;
    imageBlit.srcOffsets[1].x               = width;
    imageBlit.srcOffsets[1].y               = height;
    imageBlit.srcOffsets[1].z               = 1;
    imageBlit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.mipLevel       = i;
    imageBlit.srcSubresource.baseArrayLayer = 0;
    imageBlit.srcSubresource.layerCount     = 1;
    imageBlit.dstOffsets[0].x               = 0;
    imageBlit.dstOffsets[0].y               = 0;
    imageBlit.dstOffsets[0].z               = 0;
    imageBlit.dstOffsets[1].x               = (width > 1) ? (width /= 2) : 1;
    imageBlit.dstOffsets[1].y               = (height > 1) ? (height /= 2) : 1;
    imageBlit.dstOffsets[1].z               = 1;
    imageBlit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.mipLevel       = i + 1;
    imageBlit.dstSubresource.baseArrayLayer = 0;
    imageBlit.dstSubresource.layerCount     = 1;

    vkCmdBlitImage(
        commandBuffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageBlit,
        VK_FILTER_LINEAR);

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &imageMemoryBarrier);
  }

  imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
  imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0,
      NULL,
      0,
      NULL,
      1,
      &imageMemoryBarrier);

  ivyCode = ivySubmitOneTimeCommandBuffer(context, commandBuffer);
  ivyFreeOneTimeCommandBuffer(context, commandBuffer);
  return ivyCode;
}

IvyCode ivyCreateGraphicsTexture(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    IvyPixelFormat                format,
    void                         *data,
    IvyGraphicsTexture           *texture) {
  IvyCode ivyCode;
  IVY_MEMSET(texture, 0, sizeof(*texture));

  texture->width     = width;
  texture->height    = height;
  texture->mipLevels = ivyCalculateMipLevels(width, height);
  texture->format    = format;

  texture->image = ivyCreateVulkanImage(
      context->device,
      texture->width,
      texture->height,
      texture->mipLevels,
      VK_SAMPLE_COUNT_1_BIT,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
          VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      ivyAsVulkanFormat(texture->format));
  if (!texture->image)
    goto error;

  texture->imageView = ivyCreateVulkanImageView(
      context->device,
      texture->image,
      VK_IMAGE_ASPECT_COLOR_BIT,
      ivyAsVulkanFormat(texture->format));
  if (!texture->imageView)
    goto error;

  ivyCode = ivyAllocateAndBindGraphicsMemoryToImage(
      context,
      allocator,
      IVY_GPU_LOCAL,
      texture->image,
      &texture->memory);
  if (!texture->memory.memory)
    goto error;

  if (data) {
    ivyCode = ivyChangeVulkanImageLayout(
        context,
        texture->mipLevels,
        texture->image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    if (ivyCode)
      goto error;

    ivyCode = ivyUploadDataToVulkanImage(
        context,
        allocator,
        texture->width,
        texture->height,
        texture->format,
        data,
        texture->image);
    if (ivyCode)
      goto error;

    ivyCode = ivyGenerateVulkanImageMips(
        context,
        texture->width,
        texture->height,
        texture->mipLevels,
        texture->image);
    if (ivyCode)
      goto error;
  }

  return IVY_OK;

error:
  ivyDestroyGraphicsTexture(context, allocator, texture);
  return IVY_NO_GRAPHICS_MEMORY;
}

void ivyDestroyGraphicsTexture(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsTexture           *texture) {
  if (!context || !allocator || !texture)
    return;

  if (texture->memory.memory) {
    ivyFreeGraphicsMemory(context, allocator, &texture->memory);
    texture->memory.memory = VK_NULL_HANDLE;
  }

  if (texture->imageView) {
    vkDestroyImageView(context->device, texture->imageView, NULL);
    texture->imageView = VK_NULL_HANDLE;
  }

  if (texture->image) {
    vkDestroyImage(context->device, texture->image, NULL);
    texture->image = VK_NULL_HANDLE;
  }
}
