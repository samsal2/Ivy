#ifndef IVY_GRAPHICS_TEXTURE_H
#define IVY_GRAPHICS_TEXTURE_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyMemoryAllocator.h"

typedef struct IvyRenderer IvyRenderer;

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

IVY_API IvyCode ivyCreateGraphicsTextureFromFile(
    IvyAnyMemoryAllocator allocator, IvyRenderer *renderer, char const *path,
    IvyGraphicsTexture **texture);

IVY_API IvyCode ivyCreateGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, int32_t width, int32_t height,
    IvyPixelFormat format, void *data, IvyGraphicsTexture **texture);

IVY_API void ivyDestroyGraphicsTexture(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, IvyGraphicsTexture *texture);

#endif
