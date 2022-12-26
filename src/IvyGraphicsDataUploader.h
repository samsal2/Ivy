#ifndef IVY_GRAPHICS_DATA_UPLOADER_H
#define IVY_GRAPHICS_DATA_UPLOADER_H

#include "IvyGraphicsContext.h"
#include "IvyGraphicsMemoryAllocator.h"
#include "IvyGraphicsTexture.h"

IvyCode ivyUploadDataToVulkanImage(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    IvyPixelFormat                format,
    void                         *data,
    VkImage                       destImage);

#endif
