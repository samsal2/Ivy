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

IVY_API IvyGraphicsContext *ivyCreateGraphicsContext(
    IvyAnyMemoryAllocator allocator, IvyApplication *application);

IVY_API void ivyDestroyGraphicsContext(IvyAnyMemoryAllocator allocator,
    IvyGraphicsContext *context);

IVY_API VkCommandBuffer ivyAllocateOneTimeCommandBuffer(
    IvyGraphicsContext *context);

IVY_API IvyCode ivySubmitOneTimeCommandBuffer(IvyGraphicsContext *context,
    VkCommandBuffer commandBuffer);

IVY_API void ivyFreeOneTimeCommandBuffer(IvyGraphicsContext *context,
    VkCommandBuffer commandBuffer);

IVY_API VkCommandBuffer ivyAllocateVulkanCommandBuffer(VkDevice device,
    VkCommandPool commandPool);

#endif
