#ifndef IVY_GRAPHICS_DATA_UPLOADER_H
#define IVY_GRAPHICS_DATA_UPLOADER_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyGraphicsTexture.h"

typedef struct IvyGraphicsDevice IvyGraphicsDevice;

IVY_API IvyCode ivyUploadDataToVulkanImage(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    VkCommandPool commandPool, int32_t width, int32_t height,
    IvyPixelFormat format, void *data, VkImage image);

IVY_API IvyCode ivyUploadDataToVulkanBuffer(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    VkCommandPool commandPool, uint64_t size, void *data, VkBuffer buffer);

#endif
