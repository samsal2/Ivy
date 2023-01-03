#ifndef IVY_RENDERER_H
#define IVY_RENDERER_H

#include "IvyDummyGraphicsMemoryAllocator.h"
#include "IvyGraphicsAttachment.h"
#include "IvyGraphicsContext.h"
#include "IvyGraphicsProgram.h"
#include "IvyGraphicsTemporaryBuffer.h"
#include "IvyMemoryAllocator.h"

#define IVY_MAX_SWAPCHAIN_IMAGES 8

typedef struct IvyGraphicsFrame {
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;
  VkImage image;
  VkImageView imageView;
  VkFramebuffer framebuffer;
  VkFence inFlightFence;
  IvyGraphicsTemporaryBufferProvider temporaryBufferProvider;
} IvyGraphicsFrame;

typedef struct IvyGraphicsRenderSemaphores {
  VkSemaphore renderDoneSemaphore;
  VkSemaphore swapchainImageAvailableSemaphore;
} IvyGraphicsRenderSemaphores;

typedef struct IvyRenderer {
  IvyAnyMemoryAllocator ownerMemoryAllocator;
  IvyGraphicsContext *graphicsContext;
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
  uint32_t frameCount;
  IvyGraphicsFrame *frames;
  IvyGraphicsRenderSemaphores *renderSemaphores;
  IvyGraphicsProgram basicGraphicsProgram;
  IvyGraphicsProgram *boundGraphicsProgram;
} IvyRenderer;

IVY_API IvyRenderer *ivyCreateRenderer(IvyAnyMemoryAllocator allocator,
    IvyApplication *application);

IVY_API void ivyDestroyRenderer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer);

IVY_API IvyGraphicsFrame *ivyGetCurrentGraphicsFrame(IvyRenderer *renderer);

IVY_API IvyCode ivyRebuildGraphicsSwapchain(IvyRenderer *renderer);

IVY_API IvyCode ivyRequestGraphicsTemporaryBufferFromRenderer(
    IvyRenderer *renderer, uint64_t size,
    IvyGraphicsTemporaryBuffer *temporaryBuffer);

IVY_API void ivyBindGraphicsProgramInRenderer(IvyRenderer *renderer,
    IvyGraphicsProgram *program);

IVY_API IvyCode ivyBeginGraphicsFrame(IvyRenderer *renderer);
IVY_API IvyCode ivyEndGraphicsFrame(IvyRenderer *renderer);

#endif
