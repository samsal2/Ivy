#ifndef IVY_GRAPHICS_CONTEXT_H
#define IVY_GRAPHICS_CONTEXT_H

#include <vulkan/vulkan.h>

#include "IvyApplication.h"
#include "IvyDeclarations.h"
#include "IvyDummyMemoryAllocator.h"

#define IVY_MAX_AVAILABLE_DEVICES 8

typedef struct IvyGraphicsContext {
  IvyApplication *application;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surfaceFormat;
  VkPresentModeKHR presentMode;
  VkFormat depthFormat;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  uint32_t availableDeviceCount;
  VkPhysicalDevice availableDevices[IVY_MAX_AVAILABLE_DEVICES];
  uint32_t graphicsQueueFamilyIndex;
  uint32_t presentQueueFamilyIndex;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSampleCountFlagBits attachmentSampleCounts;
  VkCommandPool transientCommandPool;
  VkDescriptorPool globalDescriptorPool;
} IvyGraphicsContext;

IvyGraphicsContext *ivyCreateGraphicsContext(IvyAnyMemoryAllocator allocator,
    IvyApplication *application);

void ivyDestroyGraphicsContext(IvyAnyMemoryAllocator allocator,
    IvyGraphicsContext *context);

VkCommandBuffer ivyAllocateOneTimeCommandBuffer(IvyGraphicsContext *context);

IvyCode ivySubmitOneTimeCommandBuffer(IvyGraphicsContext *context,
    VkCommandBuffer commandBuffer);

void ivyFreeOneTimeCommandBuffer(IvyGraphicsContext *context,
    VkCommandBuffer commandBuffer);

VkCommandBuffer ivyAllocateVulkanCommandBuffer(VkDevice device,
    VkCommandPool commandPool);

#endif
