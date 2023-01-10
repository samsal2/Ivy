#ifndef IVY_VULKAN_UTILITIES_H
#define IVY_VULKAN_UTILITIES_H

#include <vulkan/vulkan.h>

#include "IvyDeclarations.h"

IVY_API IvyCode ivyVulkanResultAsIvyCode(VkResult vulkanResult);

IVY_API VkResult ivyCreateVulkanImage(VkDevice device, int32_t width,
    int32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples,
    VkImageUsageFlags usage, VkFormat format, VkImage *image);

IVY_API VkResult ivyCreateVulkanImageView(VkDevice device, VkImage image,
    VkImageAspectFlags aspect, VkFormat format, VkImageView *imageView);

IVY_API VkResult ivyAllocateVulkanDescriptorSet(VkDevice device,
    VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet *descriptorSet);

IVY_API VkResult ivyAllocateVulkanCommandBuffer(VkDevice device,
    VkCommandPool commandPool, VkCommandBuffer *commandBuffer);

IVY_API VkResult ivyCreateVulkanBuffer(VkDevice device,
    VkBufferUsageFlagBits flags, uint64_t size, VkBuffer *buffer);

IVY_API VkResult ivyAllocateAndBeginVulkanCommandBuffer(VkDevice device,
    VkCommandPool commandPool, VkCommandBuffer *commandBuffer);

IVY_API VkResult ivyEndSubmitAndFreeVulkanCommandBuffer(VkDevice device,
    VkQueue graphicsQueue, VkCommandPool commandPool,
    VkCommandBuffer commandBuffer);

#endif
