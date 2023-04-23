#ifndef IVY_RENDERER_H
#define IVY_RENDERER_H

#include "IvyApplication.h"
#include "IvyDummyGraphicsMemoryAllocator.h"
#include "IvyGraphicsProgram.h"
#include "IvyMemoryAllocator.h"
#include "IvyVectorMath.h"

#define IVY_MAX_SWAPCHAIN_IMAGES 8

typedef struct IvyGraphicsDevice {
  VkPhysicalDevice physicalDevice;
  VkDevice logicalDevice;
  uint32_t graphicsQueueFamilyIndex;
  uint32_t presentQueueFamilyIndex;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
} IvyGraphicsDevice;

typedef struct IvyGraphicsAttachment {
  int32_t width;
  int32_t height;
  VkImage image;
  VkImageView imageView;
  IvyGraphicsMemory memory;
} IvyGraphicsAttachment;

typedef struct IvyGraphicsRenderBufferChunk {
  uint64_t size;
  uint64_t offset;
  VkBuffer buffer;
  VkDescriptorSet descriptorSet;
  IvyGraphicsMemory memory;
} IvyGraphicsRenderBufferChunk;

typedef struct IvyGraphicsTemporaryBuffer {
  void *data;
  uint64_t offsetInU64;
  uint32_t offsetInU32;
  uint64_t size;
  VkBuffer buffer;
  VkDescriptorSet descriptorSet;
} IvyGraphicsTemporaryBuffer;

typedef struct IvyGraphicsFrame {
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;
  VkImage image;
  VkImageView imageView;
  VkFramebuffer framebuffer;
  VkFence inFlightFence;
  IvyGraphicsRenderBufferChunk currentChunk;
  uint32_t garbageChunkCount;
  IvyGraphicsRenderBufferChunk *garbageChunks;
} IvyGraphicsFrame;

typedef struct IvyGraphicsRenderSemaphores {
  VkSemaphore renderDoneSemaphore;
  VkSemaphore swapchainImageAvailableSemaphore;
} IvyGraphicsRenderSemaphores;

typedef struct IvyGraphicsEnvironmentTexture {
  VkImage image;
  VkImageView imageView;
  IvyGraphicsMemory memory;
} IvyGraphicsEnvironmentTexture;

typedef struct IvyGraphicsEnviromentMap {
  VkImage image;
  VkImageView imageView;
  IvyGraphicsMemory memory;
} IvyGraphicsEnviromentMap;

typedef struct IvyGraphicsEnviroment {
  VkDescriptorSet descriptorSet;
  IvyGraphicsEnvironmentTexture skybox;
  IvyGraphicsEnviromentMap brdflut;
  IvyGraphicsEnviromentMap irradiance;
} IvyGraphicsEnvironment;

typedef struct IvyRenderer {
  IvyM4 projection;
  IvyM4 cameraView;
  IvyV3 cameraDirection;
  IvyV3 cameraUp;
  IvyV3 cameraEye;
  IvyApplication *application;
  IvyAnyMemoryAllocator ownerMemoryAllocator;
  VkInstance instance;
  PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT;
  PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;
  VkFormat surfaceFormat;
  VkColorSpaceKHR surfaceColorspace;
  VkPresentModeKHR presentMode;
  VkFormat depthFormat;
  VkSampleCountFlagBits attachmentsSampleCounts;
  uint32_t availablePhysicalDeviceCount;
  VkPhysicalDevice *availablePhysicalDevices;
  IvyGraphicsDevice device;
  VkCommandPool transientCommandPool;
  VkDescriptorPool globalDescriptorPool;
  IvyDummyGraphicsMemoryAllocator defaultGraphicsMemoryAllocator;
  VkClearValue clearValues[2];
  VkRenderPass mainRenderPass;
  VkDescriptorSetLayout uniformDescriptorSetLayout;
  VkDescriptorSetLayout textureDescriptorSetLayout;
  VkPipelineLayout mainPipelineLayout;
  IvyGraphicsAttachment colorAttachment;
  IvyGraphicsAttachment depthAttachment;
  IvyBool requiresSwapchainRebuild;
  int32_t swapchainWidth;
  int32_t swapchainHeight;
  VkSwapchainKHR swapchain;
  uint32_t swapchainImageCount;
  uint32_t currentSwapchainImageIndex;
  uint32_t currentSemaphoreIndex;
  // IvyGraphicsEnvironment environment;
  IvyGraphicsFrame *frames;
  IvyGraphicsRenderSemaphores *renderSemaphores;
  IvyGraphicsProgram basicGraphicsProgram;
  IvyGraphicsProgram *boundGraphicsProgram;
} IvyRenderer;

IVY_API IvyCode ivyCreateRenderer(IvyAnyMemoryAllocator allocator,
    IvyApplication *application, IvyRenderer **renderer);

IVY_API void ivyDestroyRenderer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer);

IVY_API IvyGraphicsFrame *ivyGetCurrentGraphicsFrame(IvyRenderer *renderer);

IVY_API IvyCode ivyRebuildGraphicsSwapchain(IvyRenderer *renderer);

IVY_API IvyCode ivyRequestGraphicsTemporaryBuffer(IvyRenderer *renderer,
    uint64_t size, IvyGraphicsTemporaryBuffer *temporaryBuffer);

IVY_API void ivyBindGraphicsProgram(IvyRenderer *renderer,
    IvyGraphicsProgram *program);

IVY_API IvyCode ivyBeginGraphicsFrame(IvyRenderer *renderer);
IVY_API IvyCode ivyEndGraphicsFrame(IvyRenderer *renderer);

#endif
