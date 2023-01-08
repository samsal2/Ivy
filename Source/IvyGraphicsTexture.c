#include "IvyGraphicsTexture.h"

#include "IvyGraphicsDataUploader.h"
#include "IvyGraphicsTemporaryBuffer.h"
#include "IvyVulkanUtilities.h"

// TODO: custom stb_image with custom allocator
#include <stb_image.h>

#ifndef IVY_FLOOR
#include <math.h>
#define IVY_FLOOR floor
#define IVY_DEBUG_LOG2 log2
#endif

IVY_INTERNAL VkFormat ivyAsVulkanFormat(IvyPixelFormat format) {
  switch (format) {
  case IVY_R8_UNORM:
    return VK_FORMAT_R8_UNORM;

  case IVY_RGBA8_SRGB:
    return VK_FORMAT_R8G8B8A8_SRGB;
  }
}

IVY_INTERNAL uint32_t ivyCalculateMipLevels(int32_t width, int32_t height) {
  return (uint32_t)(IVY_FLOOR(IVY_DEBUG_LOG2(IVY_MAX(width, height))) + 1);
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

IVY_API IvyCode ivyChangeVulkanImageLayout(IvyGraphicsContext *context,
    uint32_t mipLevels, VkImage image, VkImageLayout sourceLayout,
    VkImageLayout destinationLayout) {
  IvyCode ivyCode;

  VkImageMemoryBarrier imageMemoryBarrier;
  VkCommandBuffer commandBuffer;

  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE_KHR;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE_KHR;

  ivyCode = ivyAllocateOneTimeCommandBuffer(context, &commandBuffer);
  if (ivyCode) {
    return ivyCode;
  }

  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.pNext = NULL;
  /* image_memory_imageMemoryBarrier.srcAccessMask = Defined later */
  /* image_memory_imageMemoryBarrier.dstAccessMask = Defined later */
  imageMemoryBarrier.oldLayout = sourceStage;
  imageMemoryBarrier.newLayout = destinationLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
  imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
  imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
  imageMemoryBarrier.subresourceRange.layerCount = 1;

  if (VK_IMAGE_LAYOUT_UNDEFINED == sourceLayout &&
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == sourceLayout &&
             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == destinationLayout) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (VK_IMAGE_LAYOUT_UNDEFINED == sourceLayout &&
             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ==
                 destinationLayout) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
    imageMemoryBarrier.dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == sourceLayout &&
             VK_IMAGE_LAYOUT_GENERAL == destinationLayout) {
    /* FIXME(samuel): is this correct? */
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  } else {
    IVY_TODO();
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
    IVY_TODO();
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
      NULL, 0, NULL, 1, &imageMemoryBarrier);

  // NOTE: even if it fails we are going to free the commandBuffer
  ivyCode = ivySubmitOneTimeCommandBuffer(context, commandBuffer);
  ivyFreeOneTimeCommandBuffer(context, commandBuffer);
  return ivyCode;
}

IVY_API IvyCode ivyGenerateVulkanImageMips(IvyGraphicsContext *context,
    int32_t width, int32_t height, uint32_t mipLevels, VkImage image) {
  uint32_t index;
  IvyCode ivyCode;
  VkImageMemoryBarrier imageMemoryBarrier;
  VkCommandBuffer commandBuffer;

  if (1 == mipLevels || 0 == mipLevels) {
    return IVY_OK;
  }

  ivyCode = ivyAllocateOneTimeCommandBuffer(context, &commandBuffer);
  if (ivyCode) {
    return ivyCode;
  }

  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.pNext = NULL;
  /* image_memory_imageMemoryBarrier.srcAccessMask =  later */
  /* image_memory_imageMemoryBarrier.dstAccessMask =  later */
  /* image_memory_imageMemoryBarrier.oldLayout =  later */
  /* image_memory_imageMemoryBarrier.newLayout =  later */
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
  imageMemoryBarrier.subresourceRange.layerCount = 1;
  imageMemoryBarrier.subresourceRange.levelCount = 1;

  for (index = 0; index < (mipLevels - 1); ++index) {
    VkImageBlit imageBlit;

    imageMemoryBarrier.subresourceRange.baseMipLevel = index;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
        &imageMemoryBarrier);

    imageBlit.srcOffsets[0].x = 0;
    imageBlit.srcOffsets[0].y = 0;
    imageBlit.srcOffsets[0].z = 0;
    imageBlit.srcOffsets[1].x = width;
    imageBlit.srcOffsets[1].y = height;
    imageBlit.srcOffsets[1].z = 1;
    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.mipLevel = index;
    imageBlit.srcSubresource.baseArrayLayer = 0;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.dstOffsets[0].x = 0;
    imageBlit.dstOffsets[0].y = 0;
    imageBlit.dstOffsets[0].z = 0;
    imageBlit.dstOffsets[1].x = (width > 1) ? (width /= 2) : 1;
    imageBlit.dstOffsets[1].y = (height > 1) ? (height /= 2) : 1;
    imageBlit.dstOffsets[1].z = 1;
    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.mipLevel = index + 1;
    imageBlit.dstSubresource.baseArrayLayer = 0;
    imageBlit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit,
        VK_FILTER_LINEAR);

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1,
        &imageMemoryBarrier);
  }

  imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
  imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1,
      &imageMemoryBarrier);

  ivyCode = ivySubmitOneTimeCommandBuffer(context, commandBuffer);
  ivyFreeOneTimeCommandBuffer(context, commandBuffer);
  return ivyCode;
}

IVY_INTERNAL VkResult ivyCreateVulkanSampler(VkDevice device,
    VkSampler *sampler) {
  VkSamplerCreateInfo samplerCreateInfo;

  samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCreateInfo.pNext = NULL;
  samplerCreateInfo.flags = 0;
  samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
#if 0
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
#else
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
#endif
  samplerCreateInfo.mipLodBias = 0.0F;
  samplerCreateInfo.anisotropyEnable = VK_TRUE;
  samplerCreateInfo.maxAnisotropy = 16.0F;
  samplerCreateInfo.compareEnable = VK_FALSE;
  samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerCreateInfo.minLod = 0.0F;
  samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

  return vkCreateSampler(device, &samplerCreateInfo, NULL, sampler);
}

IVY_INTERNAL void ivyWriteVulkanTextureDynamicDescriptorSet(VkDevice device,
    VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout,
    VkDescriptorSet descriptorSet) {
  VkDescriptorImageInfo descriptorImageInfos[2];
  VkWriteDescriptorSet writeDescriptorSets[2];

  descriptorImageInfos[0].sampler = VK_NULL_HANDLE;
  descriptorImageInfos[0].imageView = imageView;
  descriptorImageInfos[0].imageLayout = imageLayout;

  descriptorImageInfos[1].sampler = sampler;
  descriptorImageInfos[1].imageView = NULL;
  descriptorImageInfos[1].imageLayout = imageLayout;

  writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSets[0].pNext = NULL;
  writeDescriptorSets[0].dstSet = descriptorSet;
  writeDescriptorSets[0].dstBinding = 0;
  writeDescriptorSets[0].dstArrayElement = 0;
  writeDescriptorSets[0].descriptorCount = 1;
  writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  writeDescriptorSets[0].pImageInfo = &descriptorImageInfos[0];
  writeDescriptorSets[0].pBufferInfo = NULL;
  writeDescriptorSets[0].pTexelBufferView = NULL;

  writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSets[1].pNext = NULL;
  writeDescriptorSets[1].dstSet = descriptorSet;
  writeDescriptorSets[1].dstBinding = 1;
  writeDescriptorSets[1].dstArrayElement = 0;
  writeDescriptorSets[1].descriptorCount = 1;
  writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  writeDescriptorSets[1].pImageInfo = &descriptorImageInfos[1];
  writeDescriptorSets[1].pBufferInfo = NULL;
  writeDescriptorSets[1].pTexelBufferView = NULL;

  vkUpdateDescriptorSets(device, IVY_ARRAY_LENGTH(writeDescriptorSets),
      writeDescriptorSets, 0, NULL);
}

IVY_API IvyCode ivyCreateGraphicsTextureFromFile(
    IvyAnyMemoryAllocator allocator, IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    VkDescriptorSetLayout textureDescriptorSetLayout, char const *path,
    IvyGraphicsTexture **texture) {
  int width;
  int height;
  int channels;
  void *data = NULL;
  IvyCode ivyCode;

  IVY_ASSERT(context);
  IVY_ASSERT(graphicsAllocator);
  IVY_ASSERT(textureDescriptorSetLayout);
  IVY_ASSERT(path);

  data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
  if (!data) {
    return IVY_ERROR_NO_MEMORY;
  }

  ivyCode = ivyCreateGraphicsTexture(allocator, context, graphicsAllocator,
      textureDescriptorSetLayout, width, height, IVY_RGBA8_SRGB, data,
      texture);

  stbi_image_free(data);

  return ivyCode;
}

IVY_API IvyCode ivyCreateGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    VkDescriptorSetLayout textureDescriptorSetLayout, int32_t width,
    int32_t height, IvyPixelFormat format, void *data,
    IvyGraphicsTexture **texture) {
  VkResult vulkanResult;
  IvyCode ivyCode;
  IvyGraphicsTexture *currentTexture;

  currentTexture = ivyAllocateMemory(allocator, sizeof(*currentTexture));
  if (!currentTexture) {
    ivyCode = IVY_ERROR_NO_MEMORY;
    goto error;
  }

  IVY_MEMSET(currentTexture, 0, sizeof(*currentTexture));

  currentTexture->width = width;
  currentTexture->height = height;
  currentTexture->mipLevels = ivyCalculateMipLevels(width, height);
  currentTexture->format = format;

  vulkanResult = ivyCreateVulkanImage(context->device, currentTexture->width,
      currentTexture->height, currentTexture->mipLevels, VK_SAMPLE_COUNT_1_BIT,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
          VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      ivyAsVulkanFormat(currentTexture->format), &currentTexture->image);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyAllocateAndBindGraphicsMemoryToImage(context, graphicsAllocator,
      IVY_GPU_LOCAL, currentTexture->image, &currentTexture->memory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  vulkanResult = ivyCreateVulkanImageView(context->device,
      currentTexture->image, VK_IMAGE_ASPECT_COLOR_BIT,
      ivyAsVulkanFormat(currentTexture->format), &currentTexture->imageView);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  if (data) {
    ivyCode = ivyChangeVulkanImageLayout(context, currentTexture->mipLevels,
        currentTexture->image, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }

    ivyCode = ivyUploadDataToVulkanImage(context, graphicsAllocator,
        currentTexture->width, currentTexture->height, currentTexture->format,
        data, currentTexture->image);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }

    ivyCode = ivyGenerateVulkanImageMips(context, currentTexture->width,
        currentTexture->height, currentTexture->mipLevels,
        currentTexture->image);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }
  }

  vulkanResult = ivyAllocateVulkanDescriptorSet(context->device,
      context->globalDescriptorPool, textureDescriptorSetLayout,
      &currentTexture->descriptorSet);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    goto error;
  }

  vulkanResult =
      ivyCreateVulkanSampler(context->device, &currentTexture->sampler);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    goto error;
  }

  ivyWriteVulkanTextureDynamicDescriptorSet(context->device,
      currentTexture->imageView, currentTexture->sampler,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, currentTexture->descriptorSet);

  *texture = currentTexture;

  return IVY_OK;

error:
  ivyDestroyGraphicsTexture(allocator, context, graphicsAllocator,
      currentTexture);
  *texture = NULL;
  return ivyCode;
}

IVY_API void ivyDestroyGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTexture *texture) {
  if (!context || !graphicsAllocator || !texture) {
    return;
  }

  if (context->device) {
    // FIXME: stopping the device just to destroy a texture...
    vkDeviceWaitIdle(context->device);
  }

  if (texture->sampler) {
    vkDestroySampler(context->device, texture->sampler, NULL);
    texture->sampler = VK_NULL_HANDLE;
  }

  if (texture->descriptorSet) {
    vkFreeDescriptorSets(context->device, context->globalDescriptorPool, 1,
        &texture->descriptorSet);
    texture->descriptorSet = VK_NULL_HANDLE;
  }

  if (texture->memory.memory) {
    ivyFreeGraphicsMemory(context, graphicsAllocator, &texture->memory);
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

  ivyFreeMemory(allocator, texture);
}
