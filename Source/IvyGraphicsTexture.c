#include "IvyGraphicsTexture.h"

#include "IvyGraphicsDataUploader.h"
#include "IvyRenderer.h"
#include "IvyVulkanUtilities.h"

// TODO: custom stb_image with custom allocator
#include <stb_image.h>

#ifndef IVY_FLOOR
#include <math.h>
#define IVY_FLOOR floor
#define IVY_LOG2 log2
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
  return (uint32_t)(IVY_FLOOR(IVY_LOG2(IVY_MAX(width, height))) + 1);
}

IVY_API VkResult ivyChangeVulkanImageLayout(VkDevice device,
    VkQueue graphicsQueue, VkCommandPool commandPool, uint32_t mipLevels,
    VkImage image, VkImageLayout sourceLayout,
    VkImageLayout destinationLayout) {

  VkResult vulkanResult;
  VkImageMemoryBarrier imageMemoryBarrier;
  VkCommandBuffer commandBuffer;

  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE_KHR;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE_KHR;

  vulkanResult = ivyAllocateAndBeginVulkanCommandBuffer(device, commandPool,
      &commandBuffer);
  if (vulkanResult) {
    return vulkanResult;
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

  vulkanResult = ivyEndSubmitAndFreeVulkanCommandBuffer(device, graphicsQueue,
      commandPool, commandBuffer);
  if (vulkanResult) {
    return vulkanResult;
  }

  return VK_SUCCESS;
}

IVY_INTERNAL VkResult ivyGenerateVulkanImageMips(VkDevice device,
    VkQueue graphicsQueue, VkCommandPool commandPool, int32_t width,
    int32_t height, uint32_t mipLevels, VkImage image) {
  uint32_t index;
  VkResult vulkanResult;
  VkImageMemoryBarrier imageMemoryBarrier;
  VkCommandBuffer commandBuffer;

  if (1 == mipLevels || 0 == mipLevels) {
    return VK_SUCCESS;
  }

  vulkanResult = ivyAllocateAndBeginVulkanCommandBuffer(device, commandPool,
      &commandBuffer);
  if (vulkanResult) {
    return vulkanResult;
  }

  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.pNext = NULL;
  imageMemoryBarrier.srcAccessMask = 0;
  imageMemoryBarrier.dstAccessMask = 0;
  imageMemoryBarrier.oldLayout = 0;
  imageMemoryBarrier.newLayout = 0;
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

  vulkanResult = ivyEndSubmitAndFreeVulkanCommandBuffer(device, graphicsQueue,
      commandPool, commandBuffer);
  if (vulkanResult) {
    return vulkanResult;
  }

  return VK_SUCCESS;
}

// TODO(Samuel): sampler cache
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

IVY_INTERNAL void ivyWriteVulkanTextureDescriptorSet(VkDevice device,
    VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout,
    VkDescriptorSet descriptorSet) {
  VkDescriptorImageInfo descriptorImageInfo;
  VkWriteDescriptorSet writeDescriptorSets[2];

  descriptorImageInfo.sampler = sampler;
  descriptorImageInfo.imageView = imageView;
  descriptorImageInfo.imageLayout = imageLayout;

  writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSets[0].pNext = NULL;
  writeDescriptorSets[0].dstSet = descriptorSet;
  writeDescriptorSets[0].dstBinding = 0;
  writeDescriptorSets[0].dstArrayElement = 0;
  writeDescriptorSets[0].descriptorCount = 1;
  writeDescriptorSets[0].descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writeDescriptorSets[0].pImageInfo = &descriptorImageInfo;
  writeDescriptorSets[0].pBufferInfo = NULL;
  writeDescriptorSets[0].pTexelBufferView = NULL;

  vkUpdateDescriptorSets(device, 1, writeDescriptorSets, 0, NULL);
}

IVY_API IvyCode ivyCreateGraphicsTextureFromFile(
    IvyAnyMemoryAllocator allocator, IvyRenderer *renderer, char const *path,
    IvyGraphicsTexture **texture) {
  int width;
  int height;
  int channels;
  void *data = NULL;
  IvyCode ivyCode;

  IVY_ASSERT(renderer);
  IVY_ASSERT(path);

  data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
  if (!data) {
    return IVY_ERROR_NO_MEMORY;
  }

  ivyCode = ivyCreateGraphicsTexture(allocator, renderer, width, height,
      IVY_RGBA8_SRGB, data, texture);

  stbi_image_free(data);

  return ivyCode;
}

IVY_API IvyCode ivyCreateGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, int32_t width, int32_t height,
    IvyPixelFormat format, void *data, IvyGraphicsTexture **texture) {
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

  vulkanResult = ivyCreateVulkanImage(renderer->device.logicalDevice,
      currentTexture->width, currentTexture->height, currentTexture->mipLevels,
      VK_SAMPLE_COUNT_1_BIT,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
          VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      ivyAsVulkanFormat(currentTexture->format), &currentTexture->image);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyAllocateAndBindGraphicsMemoryToImage(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, IVY_GPU_LOCAL,
      currentTexture->image, &currentTexture->memory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  vulkanResult = ivyCreateVulkanImageView(renderer->device.logicalDevice,
      currentTexture->image, VK_IMAGE_ASPECT_COLOR_BIT,
      ivyAsVulkanFormat(currentTexture->format), &currentTexture->imageView);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  if (data) {
    vulkanResult = ivyChangeVulkanImageLayout(renderer->device.logicalDevice,
        renderer->device.graphicsQueue, renderer->transientCommandPool,
        currentTexture->mipLevels, currentTexture->image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }

    ivyCode = ivyUploadDataToVulkanImage(&renderer->device,
        &renderer->defaultGraphicsMemoryAllocator,
        renderer->transientCommandPool, currentTexture->width,
        currentTexture->height, currentTexture->format, data,
        currentTexture->image);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }

    vulkanResult = ivyGenerateVulkanImageMips(renderer->device.logicalDevice,
        renderer->device.graphicsQueue, renderer->transientCommandPool,
        currentTexture->width, currentTexture->height,
        currentTexture->mipLevels, currentTexture->image);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }
  }

  vulkanResult = ivyAllocateVulkanDescriptorSet(renderer->device.logicalDevice,
      renderer->globalDescriptorPool, renderer->textureDescriptorSetLayout,
      &currentTexture->descriptorSet);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    goto error;
  }

  vulkanResult = ivyCreateVulkanSampler(renderer->device.logicalDevice,
      &currentTexture->sampler);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    goto error;
  }

  ivyWriteVulkanTextureDescriptorSet(renderer->device.logicalDevice,
      currentTexture->imageView, currentTexture->sampler,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, currentTexture->descriptorSet);

  *texture = currentTexture;

  return IVY_OK;

error:
  ivyDestroyGraphicsTexture(allocator, renderer, currentTexture);
  *texture = NULL;
  return ivyCode;
}

IVY_API void ivyDestroyGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, IvyGraphicsTexture *texture) {
  if (!texture) {
    return;
  }

  if (renderer->device.logicalDevice) {
    vkDeviceWaitIdle(renderer->device.logicalDevice);
  }

  if (texture->sampler) {
    vkDestroySampler(renderer->device.logicalDevice, texture->sampler, NULL);
    texture->sampler = VK_NULL_HANDLE;
  }

  if (texture->descriptorSet) {
    vkFreeDescriptorSets(renderer->device.logicalDevice,
        renderer->globalDescriptorPool, 1, &texture->descriptorSet);
    texture->descriptorSet = VK_NULL_HANDLE;
  }

  ivyFreeGraphicsMemory(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, &texture->memory);

  if (texture->imageView) {
    vkDestroyImageView(renderer->device.logicalDevice, texture->imageView,
        NULL);
    texture->imageView = VK_NULL_HANDLE;
  }

  if (texture->image) {
    vkDestroyImage(renderer->device.logicalDevice, texture->image, NULL);
    texture->image = VK_NULL_HANDLE;
  }

  ivyFreeMemory(allocator, texture);
}
