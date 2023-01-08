#include "IvyRenderer.h"

#include "IvyGraphicsTexture.h"
#include "IvyVulkanUtilities.h"

IVY_INTERNAL VkResult ivyCreateVulkanSwapchainFramebuffer(
    VkDevice device, int32_t width, int32_t height, VkRenderPass mainRenderPass,
    VkImageView swapchainImageView, VkImageView colorAttachmentImageView,
    VkImageView depthAttachmentImageView, VkFramebuffer *framebuffer) {
  VkFramebufferCreateInfo framebufferCreateInfo;
  VkImageView attachments[3];

  attachments[0] = colorAttachmentImageView,
  attachments[1] = depthAttachmentImageView,
  attachments[2] = swapchainImageView;

  framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCreateInfo.pNext = NULL;
  framebufferCreateInfo.flags = 0;
  framebufferCreateInfo.renderPass = mainRenderPass;
  framebufferCreateInfo.attachmentCount = IVY_ARRAY_LENGTH(attachments);
  framebufferCreateInfo.pAttachments = attachments;
  framebufferCreateInfo.width = width;
  framebufferCreateInfo.height = height;
  framebufferCreateInfo.layers = 1;

  return vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, framebuffer);
}

IVY_INTERNAL VkResult ivyCreateVulkanCommandPool(
    VkDevice device, uint32_t graphicsQueueFamilyIndex,
    VkCommandPoolCreateFlagBits flags, VkCommandPool *commandPool) {
  VkCommandPoolCreateInfo commandPoolCreateInfo;

  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = NULL;
  commandPoolCreateInfo.flags = flags;
  commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

  return vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, commandPool);
}

IVY_INTERNAL VkResult ivyCreateVulkanSignaledFence(VkDevice device,
                                                   VkFence *fence) {
  VkFenceCreateInfo fenceCreateInfo;

  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.pNext = NULL;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  return vkCreateFence(device, &fenceCreateInfo, NULL, fence);
}

IVY_INTERNAL VkResult ivyCreateVulkanSemaphore(VkDevice device,
                                               VkSemaphore *semaphore) {
  VkSemaphoreCreateInfo semaphoreCreateInfo;

  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = NULL;
  semaphoreCreateInfo.flags = 0;

  return vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, semaphore);
}

IVY_INTERNAL void
ivyDestroyGraphicsFrame(IvyGraphicsContext *context,
                        IvyAnyGraphicsMemoryAllocator allocator,
                        IvyGraphicsFrame *frame) {
  ivyDestroyGraphicsTemporaryBufferProvider(context, allocator,
                                            &frame->temporaryBufferProvider);

  if (frame->inFlightFence) {
    vkDestroyFence(context->device, frame->inFlightFence, NULL);
    frame->inFlightFence = VK_NULL_HANDLE;
  }

  if (frame->framebuffer) {
    vkDestroyFramebuffer(context->device, frame->framebuffer, NULL);
    frame->framebuffer = VK_NULL_HANDLE;
  }

  if (frame->imageView) {
    vkDestroyImageView(context->device, frame->imageView, NULL);
    frame->imageView = VK_NULL_HANDLE;
  }

  if (frame->commandBuffer) {
    IVY_ASSERT(frame->commandPool);
    vkFreeCommandBuffers(context->device, frame->commandPool, 1,
                         &frame->commandBuffer);
    frame->commandBuffer = VK_NULL_HANDLE;
  }

  if (frame->commandPool) {
    vkDestroyCommandPool(context->device, frame->commandPool, NULL);
    frame->commandPool = VK_NULL_HANDLE;
  }
}

IVY_INTERNAL IvyCode ivyCreateGraphicsFrame(
    IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator, int32_t width,
    int32_t height, VkRenderPass mainRenderPass, VkImage swapchainImage,
    VkImageView colorAttachmentImageView, VkImageView depthAttachmentImageView,
    IvyGraphicsFrame *frame) {
  VkResult vulkanResult;
  IvyCode ivyCode;

  IVY_MEMSET(frame, 0, sizeof(*frame));
  IVY_ASSERT(swapchainImage);

  vulkanResult = ivyCreateVulkanCommandPool(context->device,
                                            context->graphicsQueueFamilyIndex,
                                            0, &frame->commandPool);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyAllocateVulkanCommandBuffer(
      context->device, frame->commandPool, &frame->commandBuffer);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  frame->image = swapchainImage;

  vulkanResult = ivyCreateVulkanImageView(
      context->device, frame->image, VK_IMAGE_ASPECT_COLOR_BIT,
      context->surfaceFormat.format, &frame->imageView);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanSwapchainFramebuffer(
      context->device, width, height, mainRenderPass, frame->imageView,
      colorAttachmentImageView, depthAttachmentImageView, &frame->framebuffer);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult =
      ivyCreateVulkanSignaledFence(context->device, &frame->inFlightFence);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyCreateGraphicsTemporaryBufferProvider(
      context, graphicsMemoryAllocator, &frame->temporaryBufferProvider);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  return IVY_OK;

error:
  ivyDestroyGraphicsFrame(context, graphicsMemoryAllocator, frame);
  return ivyCode;
}

IVY_INTERNAL void ivyDestroyGraphicsRenderSemaphores(
    VkDevice device, IvyGraphicsRenderSemaphores *renderSemaphores) {
  if (renderSemaphores->swapchainImageAvailableSemaphore) {
    vkDestroySemaphore(
        device, renderSemaphores->swapchainImageAvailableSemaphore, NULL);
    renderSemaphores->swapchainImageAvailableSemaphore = VK_NULL_HANDLE;
  }

  if (renderSemaphores->renderDoneSemaphore) {
    vkDestroySemaphore(device, renderSemaphores->renderDoneSemaphore, NULL);
    renderSemaphores->renderDoneSemaphore = VK_NULL_HANDLE;
  }
}

IVY_INTERNAL IvyCode ivyCreateGraphicsRenderSemaphores(
    VkDevice device, IvyGraphicsRenderSemaphores *renderSemaphores) {
  VkResult vulkanResult;
  IvyCode ivyCode;

  vulkanResult =
      ivyCreateVulkanSemaphore(device, &renderSemaphores->renderDoneSemaphore);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanSemaphore(
      device, &renderSemaphores->swapchainImageAvailableSemaphore);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  return IVY_OK;

error:
  ivyDestroyGraphicsRenderSemaphores(device, renderSemaphores);
  return ivyCode;
}

IVY_INTERNAL IvyGraphicsRenderSemaphores *
ivyAllocateAndCreateGraphicsRenderSemaphores(IvyAnyMemoryAllocator allocator,
                                             VkDevice device, uint32_t count) {
  uint32_t index;
  IvyGraphicsRenderSemaphores *semaphores;

  semaphores = ivyAllocateAndZeroMemory(allocator, count, sizeof(*semaphores));
  if (!semaphores) {
    return NULL;
  }

  for (index = 0; index < count; ++index) {
    IvyCode ivyCode;

    ivyCode = ivyCreateGraphicsRenderSemaphores(device, &semaphores[index]);
    if (ivyCode) {
      goto error;
    }
  }

  return semaphores;

error:
  for (index = 0; index < count; ++index) {
    ivyDestroyGraphicsRenderSemaphores(device, &semaphores[index]);
  }
  ivyFreeMemory(allocator, semaphores);
  return NULL;
}

IVY_INTERNAL VkImage *
ivyAllocateVulkanSwapchainImages(IvyAnyMemoryAllocator allocator,
                                 VkDevice device, VkSwapchainKHR swapchain,
                                 uint32_t *swapchainImageCount) {
  VkResult vulkanResult;
  VkImage *images;

  vulkanResult =
      vkGetSwapchainImagesKHR(device, swapchain, swapchainImageCount, NULL);
  if (vulkanResult) {
    return NULL;
  }

  images = ivyAllocateMemory(allocator, *swapchainImageCount * sizeof(*images));
  if (!images) {
    return NULL;
  }

  vulkanResult =
      vkGetSwapchainImagesKHR(device, swapchain, swapchainImageCount, images);
  if (vulkanResult) {
    ivyFreeMemory(allocator, images);
    return NULL;
  }

  return images;
}

IVY_INTERNAL IvyGraphicsFrame *ivyAllocateAndCreateGraphicsFrames(
    IvyAnyMemoryAllocator allocator, IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator, int32_t width,
    int32_t height, VkRenderPass mainRenderPass, uint32_t swapchainImageCount,
    VkImage *swapchainImages, VkImageView colorAttachmentImageView,
    VkImageView depthAttachmentImageView) {
  uint32_t index;
  IvyGraphicsFrame *frames;

  frames =
      ivyAllocateAndZeroMemory(allocator, swapchainImageCount, sizeof(*frames));
  if (!frames) {
    return NULL;
  }

  for (index = 0; index < swapchainImageCount; ++index) {
    IvyCode ivyCode = ivyCreateGraphicsFrame(
        context, graphicsAllocator, width, height, mainRenderPass,
        swapchainImages[index], colorAttachmentImageView,
        depthAttachmentImageView, &frames[index]);
    IVY_ASSERT(frames[index].inFlightFence);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }
  }

  return frames;

error:
  for (index = 0; index < swapchainImageCount; ++index) {
    ivyDestroyGraphicsFrame(context, graphicsAllocator, &frames[index]);
  }
  ivyFreeMemory(allocator, frames);
  return NULL;
}

IVY_INTERNAL VkResult ivyCreateVulkanSwapchain(
    IvyAnyMemoryAllocator allocator, IvyGraphicsContext *context,
    IvyAnyGraphicsMemoryAllocator graphicsAllocator,
    VkRenderPass mainRenderPass, uint32_t minSwapchainImageCount, int32_t width,
    int32_t height, VkImageView colorAttachmentImageView,
    VkImageView depthAttachmentImageView, uint32_t *swapchainImageCount,
    IvyGraphicsFrame **frames, IvyGraphicsRenderSemaphores **renderSemaphores,
    VkSwapchainKHR *swapchain) {
  VkResult vulkanResult;
  VkImage *swapchainImages = NULL;
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  VkSwapchainCreateInfoKHR swapchainCreateInfo;
  uint32_t queueFamilyIndices[2];

  queueFamilyIndices[0] = context->graphicsQueueFamilyIndex;
  queueFamilyIndices[1] = context->presentQueueFamilyIndex;

  *swapchainImageCount = 0;
  *frames = NULL;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      context->physicalDevice, context->surface, &surfaceCapabilities);

  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext = NULL;
  swapchainCreateInfo.flags = 0;
  swapchainCreateInfo.surface = context->surface;
  swapchainCreateInfo.minImageCount = minSwapchainImageCount;
  swapchainCreateInfo.imageFormat = context->surfaceFormat.format;
  swapchainCreateInfo.imageColorSpace = context->surfaceFormat.colorSpace;
  swapchainCreateInfo.imageExtent.width = width;
  swapchainCreateInfo.imageExtent.height = height;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = context->presentMode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  if (context->graphicsQueueFamilyIndex == context->presentQueueFamilyIndex) {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = NULL;
  } else {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  }

  vulkanResult = vkCreateSwapchainKHR(context->device, &swapchainCreateInfo,
                                      NULL, swapchain);
  if (vulkanResult) {
    goto error;
  }

  swapchainImages = ivyAllocateVulkanSwapchainImages(
      allocator, context->device, *swapchain, swapchainImageCount);
  if (!*swapchainImages) {
    goto error;
  }

  *frames = ivyAllocateAndCreateGraphicsFrames(
      allocator, context, graphicsAllocator, width, height, mainRenderPass,
      *swapchainImageCount, swapchainImages, colorAttachmentImageView,
      depthAttachmentImageView);
  if (!*frames) {
    goto error;
  }

  *renderSemaphores = ivyAllocateAndCreateGraphicsRenderSemaphores(
      allocator, context->device, *swapchainImageCount);
  if (!*renderSemaphores) {
    goto error;
  }

  ivyFreeMemory(allocator, swapchainImages);

  return VK_SUCCESS;

error:
  if (*frames) {
    uint32_t index;
    for (index = 0; index < *swapchainImageCount; ++index) {
      ivyDestroyGraphicsFrame(context, graphicsAllocator, &(*frames)[index]);
    }
    ivyFreeMemory(allocator, *frames);
    *frames = NULL;
  }

  if (swapchainImages) {
    ivyFreeMemory(allocator, swapchainImages);
  }

  if (*swapchain) {
    vkDestroySwapchainKHR(context->device, *swapchain, NULL);
  }

  *swapchainImageCount = 0;

  return vulkanResult;
}

IVY_INTERNAL VkResult ivyCreateVulkanMainRenderPass(
    VkDevice device, VkFormat colorFormat, VkFormat depthFormat,
    VkSampleCountFlagBits sampleCount, VkRenderPass *mainRenderPass) {
  VkAttachmentReference colorAttachmentReference;
  VkAttachmentReference depthAttachmentReference;
  VkAttachmentReference resolveAttachmentReference;
  VkAttachmentDescription attachmentDescriptions[3];
  VkSubpassDescription subpassDescription;
  VkSubpassDependency subpassDependency;
  VkRenderPassCreateInfo renderPassCreateInfo;

  colorAttachmentReference.attachment = 0;
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  depthAttachmentReference.attachment = 1;
  depthAttachmentReference.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  resolveAttachmentReference.attachment = 2;
  resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // color attachment
  attachmentDescriptions[0].flags = 0;
  attachmentDescriptions[0].format = colorFormat;
  attachmentDescriptions[0].samples = sampleCount;
  attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescriptions[0].finalLayout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // depth attachment
  attachmentDescriptions[1].flags = 0;
  attachmentDescriptions[1].format = depthFormat;
  attachmentDescriptions[1].samples = sampleCount;
  attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescriptions[1].finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // resolve attachment
  attachmentDescriptions[2].flags = 0;
  attachmentDescriptions[2].format = colorFormat;
  attachmentDescriptions[2].samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  subpassDescription.flags = 0;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = NULL;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorAttachmentReference;
  subpassDescription.pResolveAttachments = &resolveAttachmentReference;
  subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = NULL;

  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask = 0;
  subpassDependency.srcStageMask |=
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpassDependency.dstStageMask = 0;
  subpassDependency.dstStageMask |=
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstAccessMask = 0;
  subpassDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependency.dstAccessMask |=
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  subpassDependency.dependencyFlags = 0;

  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.pNext = NULL;
  renderPassCreateInfo.flags = 0;
  renderPassCreateInfo.attachmentCount =
      IVY_ARRAY_LENGTH(attachmentDescriptions);
  renderPassCreateInfo.pAttachments = attachmentDescriptions;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpassDescription;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &subpassDependency;

  return vkCreateRenderPass(device, &renderPassCreateInfo, NULL,
                            mainRenderPass);
}

IVY_INTERNAL VkResult ivyCreateVulkanUniformDescriptorSetLayout(
    VkDevice device, VkDescriptorSetLayout *descriptorSetLayout) {
  VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;

  descriptorSetLayoutBinding.binding = 0;
  descriptorSetLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  descriptorSetLayoutBinding.descriptorCount = 1;
  descriptorSetLayoutBinding.stageFlags = 0;
  descriptorSetLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
  descriptorSetLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
  descriptorSetLayoutBinding.pImmutableSamplers = NULL;

  descriptorSetLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutCreateInfo.pNext = NULL;
  descriptorSetLayoutCreateInfo.flags = 0;
  descriptorSetLayoutCreateInfo.bindingCount = 1;
  descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

  return vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo,
                                     NULL, descriptorSetLayout);
}

IVY_INTERNAL VkResult ivyCreateVulkanTextureDescriptorSetLayout(
    VkDevice device, VkDescriptorSetLayout *descriptorSetLayout) {
  uint32_t index;
  VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2];
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;

  for (index = 0; index < IVY_ARRAY_LENGTH(descriptorSetLayoutBindings);
       ++index) {
    descriptorSetLayoutBindings[index].binding = index;
    if (0 == index) {
      descriptorSetLayoutBindings[index].descriptorType =
          VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    } else if (1 == index) {
      descriptorSetLayoutBindings[index].descriptorType =
          VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    descriptorSetLayoutBindings[index].descriptorCount = 1;
    descriptorSetLayoutBindings[index].stageFlags = 0;
    descriptorSetLayoutBindings[index].stageFlags |=
        VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorSetLayoutBindings[index].pImmutableSamplers = NULL;
  }

  descriptorSetLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutCreateInfo.pNext = NULL;
  descriptorSetLayoutCreateInfo.flags = 0;
  descriptorSetLayoutCreateInfo.bindingCount =
      IVY_ARRAY_LENGTH(descriptorSetLayoutBindings);
  descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

  return vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo,
                                     NULL, descriptorSetLayout);
}

IVY_INTERNAL VkResult ivyCreateVulkanMainPipelineLayout(
    VkDevice device, VkDescriptorSetLayout uniformDescriptorSetLayout,
    VkDescriptorSetLayout textureDescriptorSetLayout,
    VkPipelineLayout *pipelineLayout) {
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  VkDescriptorSetLayout descriptorSetLayouts[2];

  descriptorSetLayouts[0] = uniformDescriptorSetLayout;
  descriptorSetLayouts[1] = textureDescriptorSetLayout;

  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.pNext = NULL;
  pipelineLayoutCreateInfo.flags = 0;
  pipelineLayoutCreateInfo.setLayoutCount =
      IVY_ARRAY_LENGTH(descriptorSetLayouts);
  pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreateInfo.pPushConstantRanges = NULL;

  return vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL,
                                pipelineLayout);
}

IVY_API IvyCode ivyCreateRenderer(IvyAnyMemoryAllocator allocator,
                                  IvyApplication *application,
                                  IvyRenderer **renderer) {
  IvyCode ivyCode = IVY_OK;
  VkResult vulkanResult;
  IvyRenderer *currentRenderer;

  currentRenderer = ivyAllocateMemory(allocator, sizeof(*currentRenderer));
  IVY_ASSERT(currentRenderer);
  if (!currentRenderer) {
    ivyCode = IVY_ERROR_NO_MEMORY;
    goto error;
  }

  IVY_MEMSET(currentRenderer, 0, sizeof(*currentRenderer));

  currentRenderer->ownerMemoryAllocator = allocator;

  ivyCode = ivyCreateGraphicsContext(allocator, application,
                                     &currentRenderer->graphicsContext);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  ivyCode = ivyCreateDummyGraphicsMemoryAllocator(
      currentRenderer->graphicsContext,
      &currentRenderer->defaultGraphicsMemoryAllocator);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  currentRenderer->clearValues[0].color.float32[0] = 0.0F;
  currentRenderer->clearValues[0].color.float32[1] = 0.0F;
  currentRenderer->clearValues[0].color.float32[2] = 0.0F;
  currentRenderer->clearValues[0].color.float32[3] = 1.0F;
  currentRenderer->clearValues[1].depthStencil.depth = 1.0F;
  currentRenderer->clearValues[1].depthStencil.stencil = 0.0F;

  vulkanResult = ivyCreateVulkanMainRenderPass(
      currentRenderer->graphicsContext->device,
      currentRenderer->graphicsContext->surfaceFormat.format,
      currentRenderer->graphicsContext->depthFormat,
      currentRenderer->graphicsContext->attachmentSampleCounts,
      &currentRenderer->mainRenderPass);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanUniformDescriptorSetLayout(
      currentRenderer->graphicsContext->device,
      &currentRenderer->uniformDescriptorSetLayout);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanTextureDescriptorSetLayout(
      currentRenderer->graphicsContext->device,
      &currentRenderer->textureDescriptorSetLayout);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanMainPipelineLayout(
      currentRenderer->graphicsContext->device,
      currentRenderer->uniformDescriptorSetLayout,
      currentRenderer->textureDescriptorSetLayout,
      &currentRenderer->mainPipelineLayout);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  currentRenderer->swapchainWidth =
      application->lastAddedWindow->framebufferWidth;
  currentRenderer->swapchainHeight =
      application->lastAddedWindow->framebufferHeight;

  ivyCode = ivyCreateGraphicsAttachment(
      currentRenderer->graphicsContext,
      &currentRenderer->defaultGraphicsMemoryAllocator,
      currentRenderer->swapchainWidth, currentRenderer->swapchainHeight,
      IVY_COLOR_ATTACHMENT, &currentRenderer->colorAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  ivyCode = ivyCreateGraphicsAttachment(
      currentRenderer->graphicsContext,
      &currentRenderer->defaultGraphicsMemoryAllocator,
      currentRenderer->swapchainWidth, currentRenderer->swapchainHeight,
      IVY_DEPTH_ATTACHMENT, &currentRenderer->depthAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  currentRenderer->requiresSwapchainRebuild = 0;

  vulkanResult = ivyCreateVulkanSwapchain(
      allocator, currentRenderer->graphicsContext,
      &currentRenderer->defaultGraphicsMemoryAllocator,
      currentRenderer->mainRenderPass, 2, currentRenderer->swapchainWidth,
      currentRenderer->swapchainHeight,
      currentRenderer->colorAttachment.imageView,
      currentRenderer->depthAttachment.imageView,
      &currentRenderer->swapchainImageCount, &currentRenderer->frames,
      &currentRenderer->renderSemaphores, &currentRenderer->swapchain);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  currentRenderer->frameCount = currentRenderer->swapchainImageCount;

  ivyCode = ivyCreateGraphicsProgram(
      allocator, currentRenderer->graphicsContext,
      currentRenderer->mainRenderPass, currentRenderer->mainPipelineLayout,
      currentRenderer->swapchainWidth, currentRenderer->swapchainHeight,
      "../GLSL/Basic.vert.spv", "../GLSL/Basic.frag.spv",
      IVY_POLYGON_MODE_FILL | IVY_DEPTH_ENABLE | IVY_BLEND_ENABLE |
          IVY_CULL_BACK | IVY_FRONT_FACE_COUNTERCLOCKWISE,
      &currentRenderer->basicGraphicsProgram);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  *renderer = currentRenderer;

  return IVY_OK;

error:
  ivyDestroyRenderer(allocator, currentRenderer);
  *renderer = NULL;
  return ivyCode;
}

IVY_API void ivyDestroyRenderer(IvyAnyMemoryAllocator allocator,
                                IvyRenderer *currentRenderer) {
  IVY_ASSERT(allocator == currentRenderer->ownerMemoryAllocator);

  if (currentRenderer->graphicsContext->device) {
    vkDeviceWaitIdle(currentRenderer->graphicsContext->device);
  }

  ivyDestroyGraphicsProgram(currentRenderer->graphicsContext,
                            &currentRenderer->basicGraphicsProgram);

  if (currentRenderer->renderSemaphores) {
    uint32_t index;
    for (index = 0; index < currentRenderer->frameCount; ++index) {
      ivyDestroyGraphicsRenderSemaphores(
          currentRenderer->graphicsContext->device,
          &currentRenderer->renderSemaphores[index]);
    }
    ivyFreeMemory(allocator, currentRenderer->renderSemaphores);
    currentRenderer->renderSemaphores = NULL;
  }

  if (currentRenderer->frames) {
    uint32_t index;
    for (index = 0; index < currentRenderer->frameCount; ++index) {
      ivyDestroyGraphicsFrame(currentRenderer->graphicsContext,
                              &currentRenderer->defaultGraphicsMemoryAllocator,
                              &currentRenderer->frames[index]);
    }
    ivyFreeMemory(allocator, currentRenderer->frames);
    currentRenderer->frames = NULL;
  }

  if (currentRenderer->swapchain) {
    vkDestroySwapchainKHR(currentRenderer->graphicsContext->device,
                          currentRenderer->swapchain, NULL);
    currentRenderer->swapchain = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsAttachment(currentRenderer->graphicsContext,
                               &currentRenderer->defaultGraphicsMemoryAllocator,
                               &currentRenderer->depthAttachment);

  ivyDestroyGraphicsAttachment(currentRenderer->graphicsContext,
                               &currentRenderer->defaultGraphicsMemoryAllocator,
                               &currentRenderer->colorAttachment);

  if (currentRenderer->mainPipelineLayout) {
    vkDestroyPipelineLayout(currentRenderer->graphicsContext->device,
                            currentRenderer->mainPipelineLayout, NULL);
    currentRenderer->mainPipelineLayout = VK_NULL_HANDLE;
  }

  if (currentRenderer->textureDescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(currentRenderer->graphicsContext->device,
                                 currentRenderer->textureDescriptorSetLayout,
                                 NULL);
    currentRenderer->textureDescriptorSetLayout = VK_NULL_HANDLE;
  }

  if (currentRenderer->uniformDescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(currentRenderer->graphicsContext->device,
                                 currentRenderer->uniformDescriptorSetLayout,
                                 NULL);
    currentRenderer->uniformDescriptorSetLayout = VK_NULL_HANDLE;
  }

  if (currentRenderer->mainRenderPass) {
    vkDestroyRenderPass(currentRenderer->graphicsContext->device,
                        currentRenderer->mainRenderPass, NULL);
    currentRenderer->mainRenderPass = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsMemoryAllocator(
      currentRenderer->graphicsContext,
      &currentRenderer->defaultGraphicsMemoryAllocator);

  ivyDestroyGraphicsContext(allocator, currentRenderer->graphicsContext);

  ivyFreeMemory(allocator, currentRenderer);
}

IVY_API IvyGraphicsFrame *ivyGetCurrentGraphicsFrame(IvyRenderer *renderer) {
  return &renderer->frames[renderer->currentSwapchainImageIndex];
}

IVY_INTERNAL IvyGraphicsRenderSemaphores *
ivyGetCurrentGraphicsRenderSemaphores(IvyRenderer *renderer) {
  return &renderer->renderSemaphores[renderer->currentSemaphoreIndex];
}

IVY_INTERNAL void
ivyDestroyGraphicsResourcesForSwapchainRebuild(IvyRenderer *renderer) {
  IvyAnyMemoryAllocator allocator = renderer->ownerMemoryAllocator;

  if (renderer->graphicsContext->device) {
    vkDeviceWaitIdle(renderer->graphicsContext->device);
  }

  ivyDestroyGraphicsProgram(renderer->graphicsContext,
                            &renderer->basicGraphicsProgram);

  if (renderer->renderSemaphores) {
    uint32_t index;
    for (index = 0; index < renderer->frameCount; ++index) {
      ivyDestroyGraphicsRenderSemaphores(renderer->graphicsContext->device,
                                         &renderer->renderSemaphores[index]);
    }
    ivyFreeMemory(allocator, renderer->renderSemaphores);
    renderer->renderSemaphores = NULL;
  }

  if (renderer->frames) {
    uint32_t index;
    for (index = 0; index < renderer->frameCount; ++index) {
      ivyDestroyGraphicsFrame(renderer->graphicsContext,
                              &renderer->defaultGraphicsMemoryAllocator,
                              &renderer->frames[index]);
    }
    ivyFreeMemory(allocator, renderer->frames);
    renderer->frames = NULL;
  }

  if (renderer->swapchain) {
    vkDestroySwapchainKHR(renderer->graphicsContext->device,
                          renderer->swapchain, NULL);
    renderer->swapchain = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsAttachment(renderer->graphicsContext,
                               &renderer->defaultGraphicsMemoryAllocator,
                               &renderer->depthAttachment);

  ivyDestroyGraphicsAttachment(renderer->graphicsContext,
                               &renderer->defaultGraphicsMemoryAllocator,
                               &renderer->colorAttachment);
}

IVY_API IvyCode ivyRebuildGraphicsSwapchain(IvyRenderer *renderer) {
  // FIXME(samuel): create first destroy second
  VkResult vulkanResult;
  IvyCode ivyCode;
  IvyAnyMemoryAllocator allocator = renderer->ownerMemoryAllocator;
  IvyApplication *application = renderer->graphicsContext->application;

  ivyDestroyGraphicsResourcesForSwapchainRebuild(renderer);

  renderer->requiresSwapchainRebuild = 0;
  renderer->currentSemaphoreIndex = 0;
  renderer->currentSwapchainImageIndex = 0;
  renderer->swapchainWidth = application->lastAddedWindow->framebufferWidth;
  renderer->swapchainHeight = application->lastAddedWindow->framebufferHeight;

  ivyCode = ivyCreateGraphicsAttachment(
      renderer->graphicsContext, &renderer->defaultGraphicsMemoryAllocator,
      renderer->swapchainWidth, renderer->swapchainHeight, IVY_COLOR_ATTACHMENT,
      &renderer->colorAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  ivyCode = ivyCreateGraphicsAttachment(
      renderer->graphicsContext, &renderer->defaultGraphicsMemoryAllocator,
      renderer->swapchainWidth, renderer->swapchainHeight, IVY_DEPTH_ATTACHMENT,
      &renderer->depthAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  vulkanResult = ivyCreateVulkanSwapchain(
      allocator, renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator, renderer->mainRenderPass, 2,
      renderer->swapchainWidth, renderer->swapchainHeight,
      renderer->colorAttachment.imageView, renderer->depthAttachment.imageView,
      &renderer->swapchainImageCount, &renderer->frames,
      &renderer->renderSemaphores, &renderer->swapchain);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  renderer->frameCount = renderer->swapchainImageCount;

  ivyCode = ivyCreateGraphicsProgram(
      allocator, renderer->graphicsContext, renderer->mainRenderPass,
      renderer->mainPipelineLayout, renderer->swapchainWidth,
      renderer->swapchainHeight, "../GLSL/basic.vert.spv",
      "../GLSL/basic.frag.spv",
      IVY_POLYGON_MODE_FILL | IVY_DEPTH_ENABLE | IVY_BLEND_ENABLE |
          IVY_CULL_BACK | IVY_FRONT_FACE_COUNTERCLOCKWISE,
      &renderer->basicGraphicsProgram);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  return IVY_OK;

error:
  return ivyCode;
}

IVY_INTERNAL IvyBool
ivyCheckIfVulkanSwapchainRequiresRebuild(VkResult vulkanResult) {
  return VK_SUBOPTIMAL_KHR == vulkanResult ||
         VK_ERROR_OUT_OF_DATE_KHR == vulkanResult;
}

IVY_INTERNAL IvyCode
ivyAcquireNextVulkanSwapchainImageIndex(IvyRenderer *renderer) {
  VkResult vulkanResult;
  IvyGraphicsRenderSemaphores *semaphores;

  semaphores = ivyGetCurrentGraphicsRenderSemaphores(renderer);

  IVY_ASSERT(renderer->swapchainImageCount);
  IVY_ASSERT(renderer->graphicsContext->device);
  IVY_ASSERT(renderer->swapchain);
  IVY_ASSERT(semaphores->renderDoneSemaphore);
  IVY_ASSERT(semaphores->swapchainImageAvailableSemaphore);

  // FIXME(samuel): check vulkanResult
  vulkanResult = vkAcquireNextImageKHR(
      renderer->graphicsContext->device, renderer->swapchain, (uint64_t)-1,
      semaphores->swapchainImageAvailableSemaphore, VK_NULL_HANDLE,
      &renderer->currentSwapchainImageIndex);

  renderer->requiresSwapchainRebuild =
      ivyCheckIfVulkanSwapchainRequiresRebuild(vulkanResult);

  return IVY_OK;
}

IVY_API IvyCode ivyRequestGraphicsTemporaryBufferFromRenderer(
    IvyRenderer *renderer, uint64_t size,
    IvyGraphicsTemporaryBuffer *temporaryBuffer) {
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);
  return ivyRequestGraphicsTemporaryBuffer(
      renderer->graphicsContext, &renderer->defaultGraphicsMemoryAllocator,
      &frame->temporaryBufferProvider, renderer->uniformDescriptorSetLayout,
      size, temporaryBuffer);
}

IVY_API IvyCode ivyBeginGraphicsFrame(IvyRenderer *renderer) {
  IvyCode ivyCode;
  VkResult vulkanResult;
  VkCommandBufferBeginInfo commandBufferBeginInfo;
  VkRenderPassBeginInfo renderPassBeginInfo;
  IvyGraphicsFrame *frame;

  IVY_UNUSED(ivyCode);
  IVY_UNUSED(vulkanResult);

  if (renderer->requiresSwapchainRebuild) {
    ivyRebuildGraphicsSwapchain(renderer);
  }

  ivyCode = ivyAcquireNextVulkanSwapchainImageIndex(renderer);
  IVY_ASSERT(!ivyCode);

  frame = ivyGetCurrentGraphicsFrame(renderer);

  IVY_ASSERT(frame);
  IVY_ASSERT(frame->inFlightFence);
  vulkanResult = vkWaitForFences(renderer->graphicsContext->device, 1,
                                 &frame->inFlightFence, VK_TRUE, (uint64_t)-1);
  IVY_ASSERT(!vulkanResult);

  vulkanResult = vkResetFences(renderer->graphicsContext->device, 1,
                               &frame->inFlightFence);
  IVY_ASSERT(!vulkanResult);

  vulkanResult = vkResetCommandPool(renderer->graphicsContext->device,
                                    frame->commandPool, 0);
  IVY_ASSERT(!vulkanResult);

  ivyClearGraphicsTemporaryBufferProvider(
      renderer->graphicsContext, &renderer->defaultGraphicsMemoryAllocator,
      &frame->temporaryBufferProvider);

  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  commandBufferBeginInfo.pNext = NULL;
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  commandBufferBeginInfo.pInheritanceInfo = NULL;

  vulkanResult =
      vkBeginCommandBuffer(frame->commandBuffer, &commandBufferBeginInfo);
  IVY_ASSERT(!vulkanResult);

  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.pNext = NULL;
  renderPassBeginInfo.renderPass = renderer->mainRenderPass;
  renderPassBeginInfo.framebuffer = frame->framebuffer;
  renderPassBeginInfo.renderArea.offset.x = 0;
  renderPassBeginInfo.renderArea.offset.y = 0;
  renderPassBeginInfo.renderArea.extent.width = renderer->swapchainWidth;
  renderPassBeginInfo.renderArea.extent.height = renderer->swapchainHeight;
  renderPassBeginInfo.clearValueCount = IVY_ARRAY_LENGTH(renderer->clearValues);
  renderPassBeginInfo.pClearValues = renderer->clearValues;

  vkCmdBeginRenderPass(frame->commandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  return IVY_OK;
}

IVY_API IvyCode ivyEndGraphicsFrame(IvyRenderer *renderer) {
  VkResult vulkanResult;
  VkSubmitInfo submitInfo;
  VkPresentInfoKHR presentInfo;
  VkPipelineStageFlagBits stage;
  IvyGraphicsFrame *frame;
  IvyGraphicsRenderSemaphores *semaphores;

  IVY_UNUSED(vulkanResult);

  frame = ivyGetCurrentGraphicsFrame(renderer);
  semaphores = ivyGetCurrentGraphicsRenderSemaphores(renderer);

  vkCmdEndRenderPass(frame->commandBuffer);
  vulkanResult = vkEndCommandBuffer(frame->commandBuffer);
  IVY_ASSERT(!vulkanResult);

  IVY_ASSERT(semaphores->swapchainImageAvailableSemaphore);
  IVY_ASSERT(semaphores->renderDoneSemaphore);
  IVY_ASSERT(frame->commandBuffer);
  IVY_ASSERT(renderer->graphicsContext->graphicsQueue);

  stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &semaphores->swapchainImageAvailableSemaphore;
  submitInfo.pWaitDstStageMask = &stage;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &frame->commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &semaphores->renderDoneSemaphore;

  vulkanResult = vkQueueSubmit(renderer->graphicsContext->graphicsQueue, 1,
                               &submitInfo, frame->inFlightFence);
  IVY_ASSERT(!vulkanResult);

  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &semaphores->renderDoneSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &renderer->swapchain;
  presentInfo.pImageIndices = &renderer->currentSwapchainImageIndex;
  presentInfo.pResults = NULL;

  // FIXME(samuel): check vulkanResult
  vulkanResult =
      vkQueuePresentKHR(renderer->graphicsContext->presentQueue, &presentInfo);

  ++renderer->currentSemaphoreIndex;
  if (renderer->currentSemaphoreIndex == renderer->swapchainImageCount) {
    renderer->currentSemaphoreIndex = 0;
  }

  renderer->boundGraphicsProgram = NULL;

  return IVY_OK;
}

IVY_API void ivyBindGraphicsProgramInRenderer(IvyRenderer *renderer,
                                              IvyGraphicsProgram *program) {
  if (renderer->boundGraphicsProgram != program) {
    IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);
    renderer->boundGraphicsProgram = program;
    vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      program->pipeline);
  }
}
