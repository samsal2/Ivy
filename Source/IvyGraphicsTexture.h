#ifndef IVY_GRAPHICS_TEXTURE_H
#define IVY_GRAPHICS_TEXTURE_H

#include "IvyGraphicsMemoryAllocator.h"

typedef enum IvyPixelFormat {
  IVY_RGBA8_SRGB,
  IVY_R8_UNORM
} IvyPixelFormat;

typedef struct IvyGraphicsTexture {
  int32_t           width;
  int32_t           height;
  uint32_t          mipLevels;
  IvyPixelFormat    format;
  VkImage           image;
  VkImageView       imageView;
  IvyGraphicsMemory memory;
} IvyGraphicsTexture;

VkImage ivyCreateVulkanImage(
    VkDevice              device,
    int32_t               width,
    int32_t               height,
    uint32_t              mipLevels,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags     usage,
    VkFormat              format);

VkImageView ivyCreateVulkanImageView(
    VkDevice           device,
    VkImage            image,
    VkImageAspectFlags aspect,
    VkFormat           format);

IvyCode ivyCreateGraphicsTexture(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    IvyPixelFormat                format,
    void                         *data,
    IvyGraphicsTexture           *texture);

void ivyDestroyGraphicsTexture(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsTexture           *texture);

#endif
