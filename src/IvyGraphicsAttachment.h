#ifndef IVY_GRAPHICS_ATTACHMENT_H
#define IVY_GRAPHICS_ATTACHMENT_H

#include "IvyGraphicsMemoryAllocator.h"

typedef enum IvyGraphicsAttachmentType {
  IVY_DEPTH_ATTACHMENT,
  IVY_COLOR_ATTACHMENT
} IvyGraphicsAttachmentType;

typedef struct IvyGraphicsAttachment {
  IvyGraphicsAttachmentType type;
  int32_t                   width;
  int32_t                   height;
  VkImage                   image;
  VkImageView               imageView;
  IvyGraphicsMemory         memory;
} IvyGraphicsAttachment;

IvyCode ivyCreateGraphicsAttachment(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    IvyGraphicsAttachmentType     type,
    IvyGraphicsAttachment        *attachment);

IvyCode ivyDestroyGraphicsAttachment(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsAttachment        *attachment);

#endif
