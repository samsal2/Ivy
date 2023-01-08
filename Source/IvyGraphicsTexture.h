#ifndef IVY_GRAPHICS_TEXTURE_H
#define IVY_GRAPHICS_TEXTURE_H

#include "IvyGraphicsMemoryAllocator.h"

typedef enum IvyPixelFormat {
  IVY_RGBA8_SRGB,
  IVY_R8_UNORM
} IvyPixelFormat;

typedef struct IvyGraphicsTexture {
  int32_t width;
  int32_t height;
  uint32_t mipLevels;
  IvyPixelFormat format;
  VkImage image;
  VkImageView imageView;
  IvyGraphicsMemory memory;
  VkDescriptorSet descriptorSet;
  VkSampler sampler;
} IvyGraphicsTexture;

IVY_API VkResult ivyCreateVulkanImage(VkDevice device, int32_t width,
    int32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples,
    VkImageUsageFlags usage, VkFormat format, VkImage *image);

IVY_API VkResult ivyCreateVulkanImageView(VkDevice device, VkImage image,
    VkImageAspectFlags aspect, VkFormat format, VkImageView *imageView);

IVY_API IvyCode ivyCreateGraphicsTextureFromFile(
    IvyAnyMemoryAllocator allocator, IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    VkDescriptorSetLayout textureDescriptorSetLayout, char const *path,
    IvyGraphicsTexture **texture);

IVY_API IvyCode ivyCreateGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    VkDescriptorSetLayout textureDescriptorSetLayout, int32_t width,
    int32_t height, IvyPixelFormat format, void *data,
    IvyGraphicsTexture **texture);

IVY_API void ivyDestroyGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    IvyGraphicsTexture *texture);

#endif
