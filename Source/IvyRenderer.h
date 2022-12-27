#ifndef IVY_RENDERER_H
#define IVY_RENDERER_H

#include "IvyDummyGraphicsMemoryAllocator.h"
#include "IvyGraphicsAttachment.h"
#include "IvyGraphicsContext.h"
#include "IvyGraphicsProgram.h"
#include "IvyGraphicsTemporaryBuffer.h"

#define IVY_MAX_SWAPCHAIN_IMAGES 8

typedef struct IvyRenderer {
  IvyGraphicsContext                  graphicsContext;
  IvyDummyGraphicsMemoryAllocator     defaultGraphicsMemoryAllocator;
  VkRenderPass                        mainRenderPass;
  VkDescriptorSetLayout               uniformDescriptorSetLayout;
  VkDescriptorSetLayout               textureDescriptorSetLayout;
  VkPipelineLayout                    mainPipelineLayout;
  IvyGraphicsAttachment               colorAttachment;
  IvyGraphicsAttachment               depthAttachment;
  VkSwapchainKHR                      swapchain;
  uint32_t                            swapchainImageCount;
  VkImage                            *swapchainImages;
  VkImageView                        *swapchainImageViews;
  VkFramebuffer                      *swapchainFramebuffers;
  IvyGraphicsProgram                  basicGraphicsProgram;
  IvyGraphicsProgram                 *boundGraphicsProgram;
  uint32_t                            temporaryBufferProviderCount;
  IvyGraphicsTemporaryBufferProvider *temporaryBufferProviders;
  uint32_t                            frameIndex;
  VkCommandPool                       frameCommandPool;
  uint32_t                            frameCommandBufferCount;
  VkCommandBuffer                    *frameCommandBuffers;
  VkFence                            *frameInFlightFence;
  VkSemaphore                        *renderDoneSemaphore;
  VkSemaphore                        *swapchainAvailableSamphoresb;
} IvyRenderer;

IvyCode ivyCreateRenderer(IvyApplication *application, IvyRenderer *renderer);

void ivyDestroyRenderer(IvyRenderer *renderer);

IvyGraphicsTemporaryBufferProvider *
ivyGetCurrentGraphicsTemporaryBufferProvider(IvyRenderer *renderer);

VkCommandBuffer
ivyGetCurrentVulkanCommandBufferInRenderer(IvyRenderer *renderer);

VkFramebuffer ivyGetCurrentVulkanFramebufferInRenderer(IvyRenderer *renderer);

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
