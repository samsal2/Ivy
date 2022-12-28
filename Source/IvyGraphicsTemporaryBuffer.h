#ifndef IVY_GRAPHICS_TEMPORARY_BUFFER_H
#define IVY_GRAPHICS_TEMPORARY_BUFFER_H

#include "IvyGraphicsContext.h"
#include "IvyGraphicsMemoryAllocator.h"

typedef struct IvyGraphicsTemporaryBufferProvider {
  uint64_t          currentBufferSize;
  uint64_t          currentBufferOffset;
  VkBuffer          currentBuffer;
  VkDescriptorSet   currentDescriptorSet;
  IvyGraphicsMemory currentMemory;
  int32_t           garbageBufferCount;
  VkDescriptorSet   garbageDescriptorSets[16];
  VkBuffer          garbageBuffers[16];
  IvyGraphicsMemory garbageMemories[16];
} IvyGraphicsTemporaryBufferProvider;

typedef struct IvyGraphicsTemporaryBuffer {
  uint64_t        offset;
  VkBuffer        buffer;
  VkDescriptorSet descriptorSet;
} IvyGraphicsTemporaryBuffer;

IvyCode ivyCreateGraphicsTemporaryBufferProvider(
    IvyGraphicsContext                 *context,
    IvyAnyGraphicsMemoryAllocator       allocator,
    IvyGraphicsTemporaryBufferProvider *provider);

void ivyClearGraphicsTemporaryBufferProvider(
    IvyGraphicsContext                 *context,
    IvyAnyGraphicsMemoryAllocator       allocator,
    IvyGraphicsTemporaryBufferProvider *provider);

void ivyDestroyGraphicsTemporaryBufferProvider(
    IvyGraphicsContext                 *context,
    IvyAnyGraphicsMemoryAllocator       allocator,
    IvyGraphicsTemporaryBufferProvider *provider);

// FIXME(samuel): remove descriptorSetLayout from here
IvyCode ivyRequestGraphicsTemporaryBuffer(
    IvyGraphicsContext                 *context,
    IvyAnyGraphicsMemoryAllocator       allocator,
    IvyGraphicsTemporaryBufferProvider *provider,
    VkDescriptorSetLayout               uniformDescriptorSetLayout,
    uint64_t                            size,
    IvyGraphicsTemporaryBuffer         *buffer);

#define IVY_VERTEX_BUFFER 0x0001
#define IVY_INDEX_BUFFER 0x0002
#define IVY_UNIFORM_BUFFER 0x0004
#define IVY_SOURCE_BUFFER 0x0008
#define IVY_DESTINATION_BUFFER 0x0010

VkBuffer ivyCreateVulkanBuffer(VkDevice device, uint32_t flags, uint64_t size);

#endif