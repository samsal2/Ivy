#ifndef IVY_RENDERER_H
#define IVY_RENDERER_H

#include "IvyDummyGraphicsMemoryAllocator.h"
#include "IvyGraphicsAttachment.h"
#include "IvyGraphicsContext.h"
#include "IvyGraphicsProgram.h"
#include "IvyGraphicsTemporaryBuffer.h"

#define IVY_MAX_SWAPCHAIN_IMAGES 8

typedef struct IvyGraphicsFrame {
  VkCommandPool                      commandPool;
  VkCommandBuffer                    commandBuffer;
  VkImage                            image;
  VkImageView                        imageView;
  VkFramebuffer                      framebuffer;
  VkFence                            inFlightFence;
  IvyGraphicsTemporaryBufferProvider temporaryBufferProvider;
} IvyGraphicsFrame;

typedef struct IvyGraphicsRenderSemaphores {
  VkSemaphore renderDoneSemaphore;
  VkSemaphore swapchainImageAvailableSemaphore;
} IvyGraphicsRenderSemaphores;

typedef struct IvyRenderer {
  IvyGraphicsContext              graphicsContext;
  IvyDummyGraphicsMemoryAllocator defaultGraphicsMemoryAllocator;
  VkClearValue                    clearValues[2];
  VkRenderPass                    mainRenderPass;
  VkDescriptorSetLayout           uniformDescriptorSetLayout;
  VkDescriptorSetLayout           textureDescriptorSetLayout;
  VkPipelineLayout                mainPipelineLayout;
  IvyGraphicsAttachment           colorAttachment;
  IvyGraphicsAttachment           depthAttachment;
  IvyBool                         requiresSwapchainRebuild;
  int32_t                         swapchainWidth;
  int32_t                         swapchainHeight;
  VkSwapchainKHR                  swapchain;
  uint32_t                        swapchainImageCount;
  uint32_t                        currentSwapchainImageIndex;
  uint32_t                        currentSemaphoreIndex;
  uint32_t                        frameCount;
  IvyGraphicsFrame               *frames;
  IvyGraphicsRenderSemaphores    *renderSemaphores;
  IvyGraphicsProgram              basicGraphicsProgram;
  IvyGraphicsProgram             *boundGraphicsProgram;
} IvyRenderer;

IvyCode ivyCreateRenderer(IvyApplication *application, IvyRenderer *renderer);

void ivyDestroyRenderer(IvyRenderer *renderer);

IvyGraphicsFrame *ivyGetCurrentGraphicsFrame(IvyRenderer *renderer);

IvyCode ivyRebuildGraphicsSwapchain(IvyRenderer *renderer);

IvyCode ivyRequestGraphicsTemporaryBufferFromRenderer(
    IvyRenderer                *renderer,
    uint64_t                    size,
    IvyGraphicsTemporaryBuffer *temporaryBuffer);

void ivyBindGraphicsProgramInRenderer(
    IvyRenderer        *renderer,
    IvyGraphicsProgram *program);

IvyCode ivyBeginGraphicsFrame(IvyRenderer *renderer);
IvyCode ivyEndGraphicsFrame(IvyRenderer *renderer);

#endif
