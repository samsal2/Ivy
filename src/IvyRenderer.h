#ifndef IVY_RENDERER_H
#define IVY_RENDERER_H

#include "IvyDummyGraphicsMemoryAllocator.h"
#include "IvyGraphicsAttachment.h"
#include "IvyGraphicsContext.h"

#define IVY_MAX_SWAPCHAIN_IMAGES 8

typedef struct IvyRenderer {
  IvyGraphicsContext              graphicsContext;
  IvyDummyGraphicsMemoryAllocator defaultGraphicsMemoryAllocator;
  VkRenderPass                    mainRenderPass;
  IvyGraphicsAttachment           colorAttachment;
  IvyGraphicsAttachment           depthAttachment;
  VkSwapchainKHR                  swapchain;
  uint32_t                        swapchainImageCount;
  VkImage                        *swapchainImages;
  VkImageView                    *swapchainImageViews;
  VkFramebuffer                  *swapchainFramebuffers;

} IvyRenderer;

IvyCode ivyCreateRenderer(IvyApplication *application, IvyRenderer *renderer);

void ivyDestroyRenderer(IvyRenderer *renderer);

#endif
