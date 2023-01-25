#ifndef IVY_GRAPHICS_SKYBOX_H
#define IVY_GRAPHICS_SKYBOX_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyMemoryAllocator.h"

typedef struct IvyRenderer IvyRenderer;

typedef struct IvyGraphicsSkybox {
  char path[256];
  VkImage image;
  VkImageView imageView;
  IvyGraphicsMemory memory;
  VkDescriptorSet descriptorSet;
  VkSampler sampler;
} IvyGraphicsSkybox;

IVY_API IvyCode ivyCreateGraphicsSkybox(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, char const *topFilePath, char const *bottomFilePath,
    char const *leftFilePath, char const *rightFilePath,
    char const *backFilePath, char const *fronFilePath,
    IvyGraphicsSkybox **skybox);

IVY_API void ivyDestroyGraphicsSkybox(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, IvyGraphicsSkybox *skybox);

#endif
