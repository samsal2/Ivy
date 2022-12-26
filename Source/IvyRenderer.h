#ifndef IVY_RENDERER_H
#define IVY_RENDERER_H

#include "IvyDummyGraphicsMemoryAllocator.h"
#include "IvyGraphicsAttachment.h"
#include "IvyGraphicsContext.h"
#include "IvyGraphicsProgram.h"

#define IVY_MAX_SWAPCHAIN_IMAGES 8

typedef struct IvyRenderer {
  IvyGraphicsContext              graphicsContext;
  IvyDummyGraphicsMemoryAllocator defaultGraphicsMemoryAllocator;
  VkRenderPass                    mainRenderPass;
  VkDescriptorSetLayout           uniformDescriptorSetLayout;
  VkDescriptorSetLayout           textureDescriptorSetLayout;
  VkPipelineLayout                mainPipelineLayout;
  IvyGraphicsAttachment           colorAttachment;
  IvyGraphicsAttachment           depthAttachment;
  VkSwapchainKHR                  swapchain;
  uint32_t                        swapchainImageCount;
  VkImage                        *swapchainImages;
  VkImageView                    *swapchainImageViews;
  VkFramebuffer                  *swapchainFramebuffers;
  IvyGraphicsProgram              basicGraphicsProgram;
  IvyGraphicsProgram *boundGraphicsProgram;
} IvyRenderer;

IvyCode ivyCreateRenderer(IvyApplication *application, IvyRenderer *renderer);

void ivyDestroyRenderer(IvyRenderer *renderer);

#endif
