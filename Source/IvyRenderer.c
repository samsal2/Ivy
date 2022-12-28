#include "IvyRenderer.h"

#include "IvyGraphicsTexture.h"

static IvyCode ivyVulkanResultToIvyCode(VkResult vulkanResult) {
  switch (vulkanResult) {
  default:
    return IVY_UNKNOWN_ERROR;
  }
}

static VkImageView *ivyAllocateAndCreateVulkanSwapchainImageViews(
    VkDevice device,
    VkFormat surfaceFormat,
    uint32_t swapchainImageCount,
    VkImage *swapchainImages) {
  uint32_t     i;
  VkImageView *imageViews = NULL;

  imageViews = IVY_MALLOC(swapchainImageCount * sizeof(*imageViews));
  if (!imageViews)
    return NULL;

  for (i = 0; i < swapchainImageCount; ++i)
    imageViews[i] = VK_NULL_HANDLE;

  for (i = 0; i < swapchainImageCount; ++i) {
    imageViews[i] = ivyCreateVulkanImageView(
        device,
        swapchainImages[i],
        VK_IMAGE_ASPECT_COLOR_BIT,
        surfaceFormat);
    if (!imageViews[i])
      goto error;
  }

  return imageViews;

error:
  for (i = 0; i < swapchainImageCount; ++i)
    if (imageViews[i])
      vkDestroyImageView(device, imageViews[i], NULL);

  IVY_FREE(imageViews);

  return NULL;
}

static VkFramebuffer ivyCreateVulkanSwapchainFramebuffer(
    VkDevice     device,
    int32_t      width,
    int32_t      height,
    VkRenderPass mainRenderPass,
    VkImageView  swapchainImageView,
    VkImageView  colorAttachmentImageView,
    VkImageView  depthAttachmentImageView) {
  VkResult                vulkanResult;
  VkFramebuffer           framebuffer;
  VkFramebufferCreateInfo framebufferCreateInfo;

  VkImageView attachments[] = {
      colorAttachmentImageView,
      depthAttachmentImageView,
      swapchainImageView};

  framebufferCreateInfo.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCreateInfo.pNext      = NULL;
  framebufferCreateInfo.flags      = 0;
  framebufferCreateInfo.renderPass = mainRenderPass;
  framebufferCreateInfo.attachmentCount = IVY_ARRAY_LENGTH(attachments);
  framebufferCreateInfo.pAttachments    = attachments;
  framebufferCreateInfo.width           = width;
  framebufferCreateInfo.height          = height;
  framebufferCreateInfo.layers          = 1;

  vulkanResult = vkCreateFramebuffer(
      device,
      &framebufferCreateInfo,
      NULL,
      &framebuffer);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return framebuffer;
}

static VkFramebuffer *ivyAllocateAndCreateVulkanSwapchainFramebuffers(
    VkDevice     device,
    uint32_t     swapchainImageCount,
    VkImageView *swapchainImageViews,
    int32_t      width,
    int32_t      height,
    VkRenderPass mainRenderPass,
    VkImageView  colorAttachmentImageView,
    VkImageView  depthAttachmentImageView) {
  uint32_t       i;
  VkFramebuffer *framebuffers = NULL;

  framebuffers = IVY_MALLOC(swapchainImageCount * sizeof(*framebuffers));
  if (!framebuffers)
    return NULL;

  for (i = 0; i < swapchainImageCount; ++i)
    framebuffers[i] = VK_NULL_HANDLE;

  for (i = 0; i < swapchainImageCount; ++i) {
    framebuffers[i] = ivyCreateVulkanSwapchainFramebuffer(
        device,
        width,
        height,
        mainRenderPass,
        swapchainImageViews[i],
        colorAttachmentImageView,
        depthAttachmentImageView);
    if (!framebuffers[i])
      goto error;
  }

  return framebuffers;

error:
  for (i = 0; i < swapchainImageCount; ++i)
    if (framebuffers[i])
      vkDestroyFramebuffer(device, framebuffers[i], NULL);

  IVY_FREE(framebuffers);

  return NULL;
}

VkCommandPool ivyCreateVulkanCommandPool(
    VkDevice                    device,
    uint32_t                    graphicsQueueFamilyIndex,
    VkCommandPoolCreateFlagBits flags) {
  VkResult                vulkanResult;
  VkCommandPool           commandPool;
  VkCommandPoolCreateInfo commandPoolCreateInfo;

  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = NULL;
  commandPoolCreateInfo.flags = flags;
  commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

  vulkanResult = vkCreateCommandPool(
      device,
      &commandPoolCreateInfo,
      NULL,
      &commandPool);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return commandPool;
}

static VkFence ivyCreateVulkanSignaledFence(VkDevice device) {
  VkResult          vulkanResult;
  VkFence           fence;
  VkFenceCreateInfo fenceCreateInfo;

  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.pNext = NULL;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  vulkanResult = vkCreateFence(device, &fenceCreateInfo, NULL, &fence);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return fence;
}

static VkSemaphore ivyCreateVulkanSemaphore(VkDevice device) {
  VkResult              vulkanResult;
  VkSemaphore           semaphore;
  VkSemaphoreCreateInfo semaphoreCreateInfo;

  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = NULL;
  semaphoreCreateInfo.flags = 0;

  vulkanResult = vkCreateSemaphore(
      device,
      &semaphoreCreateInfo,
      NULL,
      &semaphore);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return semaphore;
}

static void ivyDestroyGraphicsFrame(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    IvyGraphicsFrame             *frame) {
  ivyDestroyGraphicsTemporaryBufferProvider(
      context,
      allocator,
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
    vkFreeCommandBuffers(
        context->device,
        frame->commandPool,
        1,
        &frame->commandBuffer);
    frame->commandBuffer = VK_NULL_HANDLE;
  }

  if (frame->commandPool) {
    vkDestroyCommandPool(context->device, frame->commandPool, NULL);
    frame->commandPool = VK_NULL_HANDLE;
  }
}

static IvyCode ivyCreateGraphicsFrame(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    VkRenderPass                  mainRenderPass,
    VkImage                       swapchainImage,
    VkImageView                   colorAttachmentImageView,
    VkImageView                   depthAttachmentImageView,
    IvyGraphicsFrame             *frame) {
  IvyCode  ivyCode;
  VkResult vulkanResult;

  IVY_MEMSET(frame, 0, sizeof(*frame));

  if (!swapchainImage)
    return IVY_INVALID_VALUE;

  frame->commandPool = ivyCreateVulkanCommandPool(
      context->device,
      context->graphicsQueueFamilyIndex,
      0);
  IVY_ASSERT(frame->commandPool);
  if (!frame->commandPool)
    goto error;

  frame->commandBuffer = ivyAllocateVulkanCommandBuffer(
      context->device,
      frame->commandPool);
  IVY_ASSERT(frame->commandBuffer);
  if (!frame->commandBuffer)
    goto error;

  frame->image = swapchainImage;

  frame->imageView = ivyCreateVulkanImageView(
      context->device,
      frame->image,
      VK_IMAGE_ASPECT_COLOR_BIT,
      context->surfaceFormat.format);
  IVY_ASSERT(frame->imageView);
  if (!frame->imageView)
    goto error;

  frame->framebuffer = ivyCreateVulkanSwapchainFramebuffer(
      context->device,
      width,
      height,
      mainRenderPass,
      frame->imageView,
      colorAttachmentImageView,
      depthAttachmentImageView);
  IVY_ASSERT(frame->framebuffer);
  if (!frame->framebuffer)
    goto error;

  frame->inFlightFence = ivyCreateVulkanSignaledFence(context->device);
  IVY_ASSERT(frame->inFlightFence);
  if (!frame->inFlightFence)
    goto error;

  ivyCode = ivyCreateGraphicsTemporaryBufferProvider(
      context,
      allocator,
      &frame->temporaryBufferProvider);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  return IVY_OK;

error:
  ivyDestroyGraphicsFrame(context, allocator, frame);
  return IVY_NO_GRAPHICS_MEMORY;
}

static void ivyDestroyGraphicsRenderSemaphores(
    VkDevice                     device,
    IvyGraphicsRenderSemaphores *renderSemaphores) {
  if (renderSemaphores->swapchainImageAvailableSemaphore) {
    vkDestroySemaphore(
        device,
        renderSemaphores->swapchainImageAvailableSemaphore,
        NULL);
    renderSemaphores->swapchainImageAvailableSemaphore = VK_NULL_HANDLE;
  }

  if (renderSemaphores->renderDoneSemaphore) {
    vkDestroySemaphore(device, renderSemaphores->renderDoneSemaphore, NULL);
    renderSemaphores->renderDoneSemaphore = VK_NULL_HANDLE;
  }
}

static IvyCode ivyCreateGraphicsRenderSemaphores(
    VkDevice                     device,
    IvyGraphicsRenderSemaphores *renderSemaphores) {
  renderSemaphores->renderDoneSemaphore = ivyCreateVulkanSemaphore(device);
  if (!renderSemaphores->renderDoneSemaphore)
    goto error;

  renderSemaphores->swapchainImageAvailableSemaphore = ivyCreateVulkanSemaphore(
      device);
  if (!renderSemaphores->swapchainImageAvailableSemaphore)
    goto error;

  return IVY_OK;

error:
  ivyDestroyGraphicsRenderSemaphores(device, renderSemaphores);
  return IVY_NO_GRAPHICS_MEMORY;
}

static IvyGraphicsRenderSemaphores *
ivyAllocateAndCreateGraphicsRenderSemaphores(VkDevice device, uint32_t count) {
  uint32_t                     i;
  IvyGraphicsRenderSemaphores *semaphores;

  semaphores = IVY_CALLOC(count, sizeof(*semaphores));
  if (!semaphores)
    return NULL;

  for (i = 0; i < count; ++i) {
    IvyCode ivyCode;

    ivyCode = ivyCreateGraphicsRenderSemaphores(device, &semaphores[i]);
    if (ivyCode)
      goto error;
  }

  return semaphores;

error:
  for (i = 0; i < count; ++i)
    ivyDestroyGraphicsRenderSemaphores(device, &semaphores[i]);
  IVY_FREE(semaphores);
  return NULL;
}

static VkImage *ivyAllocateVulkanSwapchainImages(
    VkDevice       device,
    VkSwapchainKHR swapchain,
    uint32_t      *swapchainImageCount) {
  VkResult vulkanResult;
  VkImage *images;

  vulkanResult = vkGetSwapchainImagesKHR(
      device,
      swapchain,
      swapchainImageCount,
      NULL);
  if (vulkanResult)
    return NULL;

  images = IVY_MALLOC(*swapchainImageCount * sizeof(*images));
  if (!images)
    return NULL;

  vulkanResult = vkGetSwapchainImagesKHR(
      device,
      swapchain,
      swapchainImageCount,
      images);
  if (vulkanResult) {
    IVY_FREE(images);
    return NULL;
  }

  return images;
}

static IvyGraphicsFrame *ivyAllocateAndCreateGraphicsFrames(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    int32_t                       width,
    int32_t                       height,
    VkRenderPass                  mainRenderPass,
    uint32_t                      swapchainImageCount,
    VkImage                      *swapchainImages,
    VkImageView                   colorAttachmentImageView,
    VkImageView                   depthAttachmentImageView) {
  uint32_t          i;
  IvyGraphicsFrame *frames;

  frames = IVY_CALLOC(swapchainImageCount, sizeof(*frames));
  if (!frames)
    return NULL;

  for (i = 0; i < swapchainImageCount; ++i) {
    IvyCode ivyCode = ivyCreateGraphicsFrame(
        context,
        allocator,
        width,
        height,
        mainRenderPass,
        swapchainImages[i],
        colorAttachmentImageView,
        depthAttachmentImageView,
        &frames[i]);
    IVY_ASSERT(frames[i].inFlightFence);
    IVY_ASSERT(!ivyCode);
    if (ivyCode)
      goto error;
  }

  return frames;

error:
  for (i = 0; i < swapchainImageCount; ++i)
    ivyDestroyGraphicsFrame(context, allocator, &frames[i]);
  IVY_FREE(frames);
  return NULL;
}

static VkSwapchainKHR ivyCreateVulkanSwapchain(
    IvyGraphicsContext           *context,
    IvyAnyGraphicsMemoryAllocator allocator,
    VkRenderPass                  mainRenderPass,
    uint32_t                      minSwapchainImageCount,
    int32_t                       width,
    int32_t                       height,
    VkImageView                   colorAttachmentImageView,
    VkImageView                   depthAttachmentImageView,
    uint32_t                     *swapchainImageCount,
    IvyGraphicsFrame            **frames,
    IvyGraphicsRenderSemaphores **renderSemaphores) {
  VkResult                 vulkanResult;
  VkImage                 *swapchainImages = NULL;
  VkSwapchainKHR           swapchain       = VK_NULL_HANDLE;
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  VkSwapchainCreateInfoKHR swapchainCreateInfo;

  uint32_t queueFamilyIndices[2] = {
      context->graphicsQueueFamilyIndex,
      context->presentQueueFamilyIndex};

  *swapchainImageCount = 0;
  *frames              = NULL;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      context->physicalDevice,
      context->surface,
      &surfaceCapabilities);

  swapchainCreateInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext   = NULL;
  swapchainCreateInfo.flags   = 0;
  swapchainCreateInfo.surface = context->surface;
  swapchainCreateInfo.minImageCount      = minSwapchainImageCount;
  swapchainCreateInfo.imageFormat        = context->surfaceFormat.format;
  swapchainCreateInfo.imageColorSpace    = context->surfaceFormat.colorSpace;
  swapchainCreateInfo.imageExtent.width  = width;
  swapchainCreateInfo.imageExtent.height = height;
  swapchainCreateInfo.imageArrayLayers   = 1;
  swapchainCreateInfo.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.preTransform       = surfaceCapabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode        = context->presentMode;
  swapchainCreateInfo.clipped            = VK_TRUE;
  swapchainCreateInfo.oldSwapchain       = VK_NULL_HANDLE;

  if (context->graphicsQueueFamilyIndex == context->presentQueueFamilyIndex) {
    swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices   = NULL;
  } else {
    swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
  }

  vulkanResult = vkCreateSwapchainKHR(
      context->device,
      &swapchainCreateInfo,
      NULL,
      &swapchain);
  if (vulkanResult)
    goto error;

  swapchainImages = ivyAllocateVulkanSwapchainImages(
      context->device,
      swapchain,
      swapchainImageCount);
  if (!*swapchainImages)
    goto error;

  *frames = ivyAllocateAndCreateGraphicsFrames(
      context,
      allocator,
      width,
      height,
      mainRenderPass,
      *swapchainImageCount,
      swapchainImages,
      colorAttachmentImageView,
      depthAttachmentImageView);
  if (!*frames)
    goto error;

  *renderSemaphores = ivyAllocateAndCreateGraphicsRenderSemaphores(
      context->device,
      *swapchainImageCount);
  if (!*renderSemaphores)
    goto error;

  return swapchain;

error:
  if (*frames) {
    uint32_t i;
    for (i = 0; i < *swapchainImageCount; ++i)
      ivyDestroyGraphicsFrame(context, allocator, &(*frames)[i]);
    IVY_FREE(*frames);
    *frames = NULL;
  }

  if (swapchainImages)
    IVY_FREE(swapchainImages);

  if (swapchain)
    vkDestroySwapchainKHR(context->device, swapchain, NULL);

  *swapchainImageCount = 0;
  return VK_NULL_HANDLE;
}

static VkRenderPass ivyCreateVulkanMainRenderPass(
    VkDevice              device,
    VkFormat              colorFormat,
    VkFormat              depthFormat,
    VkSampleCountFlagBits sampleCount) {
  VkResult                vulkanResult;
  VkRenderPass            mainRenderPass;
  VkAttachmentReference   colorAttachmentReference;
  VkAttachmentReference   depthAttachmentReference;
  VkAttachmentReference   resolveAttachmentReference;
  VkAttachmentDescription attachmentDescriptions[3];
  VkSubpassDescription    subpassDescription;
  VkSubpassDependency     subpassDependency;
  VkRenderPassCreateInfo  renderPassCreateInfo;

  colorAttachmentReference.attachment = 0;
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  depthAttachmentReference.attachment = 1;
  depthAttachmentReference
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  resolveAttachmentReference.attachment = 2;
  resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // color attachment
  attachmentDescriptions[0].flags          = 0;
  attachmentDescriptions[0].format         = colorFormat;
  attachmentDescriptions[0].samples        = sampleCount;
  attachmentDescriptions[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescriptions[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescriptions[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescriptions[0]
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // depth attachment
  attachmentDescriptions[1].flags          = 0;
  attachmentDescriptions[1].format         = depthFormat;
  attachmentDescriptions[1].samples        = sampleCount;
  attachmentDescriptions[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescriptions[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescriptions[1]
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // resolve attachment
  attachmentDescriptions[2].flags          = 0;
  attachmentDescriptions[2].format         = colorFormat;
  attachmentDescriptions[2].samples        = VK_SAMPLE_COUNT_1_BIT;
  attachmentDescriptions[2].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[2].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescriptions[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescriptions[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescriptions[2].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  subpassDescription.flags                   = 0;
  subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.inputAttachmentCount    = 0;
  subpassDescription.pInputAttachments       = NULL;
  subpassDescription.colorAttachmentCount    = 1;
  subpassDescription.pColorAttachments       = &colorAttachmentReference;
  subpassDescription.pResolveAttachments     = &resolveAttachmentReference;
  subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments    = NULL;

  subpassDependency.srcSubpass   = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass   = 0;
  subpassDependency.srcStageMask = 0;
  subpassDependency
      .srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpassDependency.dstStageMask = 0;
  subpassDependency
      .dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstAccessMask = 0;
  subpassDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependency
      .dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  subpassDependency.dependencyFlags = 0;

  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.pNext = NULL;
  renderPassCreateInfo.flags = 0;
  renderPassCreateInfo.attachmentCount = IVY_ARRAY_LENGTH(
      attachmentDescriptions);
  renderPassCreateInfo.pAttachments    = attachmentDescriptions;
  renderPassCreateInfo.subpassCount    = 1;
  renderPassCreateInfo.pSubpasses      = &subpassDescription;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies   = &subpassDependency;

  vulkanResult = vkCreateRenderPass(
      device,
      &renderPassCreateInfo,
      NULL,
      &mainRenderPass);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return mainRenderPass;
}

static VkDescriptorSetLayout
ivyCreateVulkanUniformDescriptorSetLayout(VkDevice device) {
  VkResult                        vulkanResult;
  VkDescriptorSetLayout           descriptorSetLayout;
  VkDescriptorSetLayoutBinding    descriptorSetLayoutBinding;
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;

  descriptorSetLayoutBinding.binding = 0;
  descriptorSetLayoutBinding
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  descriptorSetLayoutBinding.descriptorCount = 1;
  descriptorSetLayoutBinding.stageFlags      = 0;
  descriptorSetLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
  descriptorSetLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
  descriptorSetLayoutBinding.pImmutableSamplers = NULL;

  descriptorSetLayoutCreateInfo
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutCreateInfo.pNext        = NULL;
  descriptorSetLayoutCreateInfo.flags        = 0;
  descriptorSetLayoutCreateInfo.bindingCount = 1;
  descriptorSetLayoutCreateInfo.pBindings    = &descriptorSetLayoutBinding;

  vulkanResult = vkCreateDescriptorSetLayout(
      device,
      &descriptorSetLayoutCreateInfo,
      NULL,
      &descriptorSetLayout);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return descriptorSetLayout;
}

static VkDescriptorSetLayout
ivyCreateVulkanTextureDescriptorSetLayout(VkDevice device) {
  uint32_t                        i;
  VkResult                        vulkanResult;
  VkDescriptorSetLayout           descriptorSetLayout;
  VkDescriptorSetLayoutBinding    descriptorSetLayoutBindings[2];
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;

  for (i = 0; i < IVY_ARRAY_LENGTH(descriptorSetLayoutBindings); ++i) {
    descriptorSetLayoutBindings[i].binding = i;
    if (0 == i)
      descriptorSetLayoutBindings[i]
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    else if (1 == i)
      descriptorSetLayoutBindings[i]
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorSetLayoutBindings[i].descriptorCount = 1;
    descriptorSetLayoutBindings[i].stageFlags      = 0;
    descriptorSetLayoutBindings[i].stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorSetLayoutBindings[i].pImmutableSamplers = NULL;
  }

  descriptorSetLayoutCreateInfo
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutCreateInfo.pNext        = NULL;
  descriptorSetLayoutCreateInfo.flags        = 0;
  descriptorSetLayoutCreateInfo.bindingCount = IVY_ARRAY_LENGTH(
      descriptorSetLayoutBindings);
  descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

  vulkanResult = vkCreateDescriptorSetLayout(
      device,
      &descriptorSetLayoutCreateInfo,
      NULL,
      &descriptorSetLayout);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return descriptorSetLayout;
}

static VkPipelineLayout ivyCreateVulkanMainPipelineLayout(
    VkDevice              device,
    VkDescriptorSetLayout uniformDescriptorSetLayout,
    VkDescriptorSetLayout textureDescriptorSetLayout) {
  VkResult                   vulkanResult;
  VkPipelineLayout           pipelineLayout;
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  VkDescriptorSetLayout      descriptorSetLayouts[] = {
      uniformDescriptorSetLayout,
      textureDescriptorSetLayout};

  pipelineLayoutCreateInfo
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.pNext          = NULL;
  pipelineLayoutCreateInfo.flags          = 0;
  pipelineLayoutCreateInfo.setLayoutCount = IVY_ARRAY_LENGTH(
      descriptorSetLayouts);
  pipelineLayoutCreateInfo.pSetLayouts            = descriptorSetLayouts;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreateInfo.pPushConstantRanges    = NULL;

  vulkanResult = vkCreatePipelineLayout(
      device,
      &pipelineLayoutCreateInfo,
      NULL,
      &pipelineLayout);
  if (vulkanResult)
    return VK_NULL_HANDLE;

  return pipelineLayout;
}

IvyCode ivyCreateRenderer(IvyApplication *application, IvyRenderer *renderer) {
  IvyCode ivyCode;

  IVY_MEMSET(renderer, 0, sizeof(*renderer));

  ivyCode = ivyCreateGraphicsContext(application, &renderer->graphicsContext);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  ivyCode = ivyCreateDummyGraphicsMemoryAllocator(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  renderer->clearValues[0].color.float32[0]     = 0.0F;
  renderer->clearValues[0].color.float32[1]     = 0.0F;
  renderer->clearValues[0].color.float32[2]     = 0.0F;
  renderer->clearValues[0].color.float32[3]     = 1.0F;
  renderer->clearValues[1].depthStencil.depth   = 1.0F;
  renderer->clearValues[1].depthStencil.stencil = 0.0F;

  renderer->mainRenderPass = ivyCreateVulkanMainRenderPass(
      renderer->graphicsContext.device,
      renderer->graphicsContext.surfaceFormat.format,
      renderer->graphicsContext.depthFormat,
      renderer->graphicsContext.attachmentSampleCounts);
  IVY_ASSERT(renderer->mainRenderPass);
  if (!renderer->mainRenderPass)
    goto error;

  renderer
      ->uniformDescriptorSetLayout = ivyCreateVulkanUniformDescriptorSetLayout(
      renderer->graphicsContext.device);
  IVY_ASSERT(renderer->mainRenderPass);
  if (!renderer->uniformDescriptorSetLayout)
    goto error;

  renderer
      ->textureDescriptorSetLayout = ivyCreateVulkanTextureDescriptorSetLayout(
      renderer->graphicsContext.device);
  IVY_ASSERT(renderer->textureDescriptorSetLayout);
  if (!renderer->textureDescriptorSetLayout)
    goto error;

  renderer->mainPipelineLayout = ivyCreateVulkanMainPipelineLayout(
      renderer->graphicsContext.device,
      renderer->uniformDescriptorSetLayout,
      renderer->textureDescriptorSetLayout);
  IVY_ASSERT(renderer->mainPipelineLayout);
  if (!renderer->mainPipelineLayout)
    goto error;

  renderer->swapchainWidth  = application->lastAddedWindow->framebufferWidth;
  renderer->swapchainHeight = application->lastAddedWindow->framebufferHeight;

  ivyCode = ivyCreateGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      IVY_COLOR_ATTACHMENT,
      &renderer->colorAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  ivyCode = ivyCreateGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      IVY_DEPTH_ATTACHMENT,
      &renderer->depthAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  renderer->requiresSwapchainRebuild = 0;

  renderer->swapchain = ivyCreateVulkanSwapchain(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->mainRenderPass,
      2,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      renderer->colorAttachment.imageView,
      renderer->depthAttachment.imageView,
      &renderer->swapchainImageCount,
      &renderer->frames,
      &renderer->renderSemaphores);
  IVY_ASSERT(renderer->swapchain);
  if (!renderer->swapchain)
    goto error;

  renderer->frameCount = renderer->swapchainImageCount;

  ivyCode = ivyCreateGraphicsProgram(
      &renderer->graphicsContext,
      renderer->mainRenderPass,
      renderer->mainPipelineLayout,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      "../GLSL/basic.vert.spv",
      "../GLSL/basic.frag.spv",
      IVY_POLYGON_MODE_FILL | IVY_DEPTH_ENABLE | IVY_BLEND_ENABLE |
          IVY_CULL_FRONT | IVY_FRONT_FACE_COUNTERCLOCKWISE,
      &renderer->basicGraphicsProgram);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  return IVY_OK;

error:
  ivyDestroyRenderer(renderer);
  return ivyCode;
}

void ivyDestroyRenderer(IvyRenderer *renderer) {
  if (renderer->graphicsContext.device)
    vkDeviceWaitIdle(renderer->graphicsContext.device);

  ivyDestroyGraphicsProgram(
      &renderer->graphicsContext,
      &renderer->basicGraphicsProgram);

  if (renderer->renderSemaphores) {
    uint32_t i;
    for (i = 0; i < renderer->frameCount; ++i)
      ivyDestroyGraphicsRenderSemaphores(
          renderer->graphicsContext.device,
          &renderer->renderSemaphores[i]);
    IVY_FREE(renderer->renderSemaphores);
    renderer->renderSemaphores = NULL;
  }

  if (renderer->frames) {
    uint32_t i;
    for (i = 0; i < renderer->frameCount; ++i)
      ivyDestroyGraphicsFrame(
          &renderer->graphicsContext,
          &renderer->defaultGraphicsMemoryAllocator,
          &renderer->frames[i]);
    IVY_FREE(renderer->frames);
    renderer->frames = NULL;
  }

  if (renderer->swapchain) {
    vkDestroySwapchainKHR(
        renderer->graphicsContext.device,
        renderer->swapchain,
        NULL);
    renderer->swapchain = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      &renderer->depthAttachment);

  ivyDestroyGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      &renderer->colorAttachment);

  if (renderer->mainPipelineLayout) {
    vkDestroyPipelineLayout(
        renderer->graphicsContext.device,
        renderer->mainPipelineLayout,
        NULL);
    renderer->mainPipelineLayout = VK_NULL_HANDLE;
  }

  if (renderer->textureDescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(
        renderer->graphicsContext.device,
        renderer->textureDescriptorSetLayout,
        NULL);
    renderer->textureDescriptorSetLayout = VK_NULL_HANDLE;
  }

  if (renderer->uniformDescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(
        renderer->graphicsContext.device,
        renderer->uniformDescriptorSetLayout,
        NULL);
    renderer->uniformDescriptorSetLayout = VK_NULL_HANDLE;
  }

  if (renderer->mainRenderPass) {
    vkDestroyRenderPass(
        renderer->graphicsContext.device,
        renderer->mainRenderPass,
        NULL);
    renderer->mainRenderPass = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsMemoryAllocator(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator);

  ivyDestroyGraphicsContext(&renderer->graphicsContext);
}

IvyGraphicsFrame *ivyGetCurrentGraphicsFrame(IvyRenderer *renderer) {
  return &renderer->frames[renderer->currentSwapchainImageIndex];
}

static IvyGraphicsRenderSemaphores *
ivyGetCurrentGraphicsRenderSemaphores(IvyRenderer *renderer) {
  return &renderer->renderSemaphores[renderer->currentSemaphoreIndex];
}

static void
ivyDestroyGraphicsResourcesForSwapchainRebuild(IvyRenderer *renderer) {
  if (renderer->graphicsContext.device)
    vkDeviceWaitIdle(renderer->graphicsContext.device);

  ivyDestroyGraphicsProgram(
      &renderer->graphicsContext,
      &renderer->basicGraphicsProgram);

  if (renderer->renderSemaphores) {
    uint32_t i;
    for (i = 0; i < renderer->frameCount; ++i)
      ivyDestroyGraphicsRenderSemaphores(
          renderer->graphicsContext.device,
          &renderer->renderSemaphores[i]);
    IVY_FREE(renderer->renderSemaphores);
    renderer->renderSemaphores = NULL;
  }

  if (renderer->frames) {
    uint32_t i;
    for (i = 0; i < renderer->frameCount; ++i)
      ivyDestroyGraphicsFrame(
          &renderer->graphicsContext,
          &renderer->defaultGraphicsMemoryAllocator,
          &renderer->frames[i]);
    IVY_FREE(renderer->frames);
    renderer->frames = NULL;
  }

  if (renderer->swapchain) {
    vkDestroySwapchainKHR(
        renderer->graphicsContext.device,
        renderer->swapchain,
        NULL);
    renderer->swapchain = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      &renderer->depthAttachment);

  ivyDestroyGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      &renderer->colorAttachment);
}

IvyCode ivyRebuildGraphicsSwapchain(IvyRenderer *renderer) {
  // FIXME(samuel): create first destroy second
  IvyCode         ivyCode;
  IvyApplication *application = renderer->graphicsContext.application;

  ivyDestroyGraphicsResourcesForSwapchainRebuild(renderer);

  renderer->swapchainWidth  = application->lastAddedWindow->framebufferWidth;
  renderer->swapchainHeight = application->lastAddedWindow->framebufferHeight;

  ivyCode = ivyCreateGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      IVY_COLOR_ATTACHMENT,
      &renderer->colorAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  ivyCode = ivyCreateGraphicsAttachment(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      IVY_DEPTH_ATTACHMENT,
      &renderer->depthAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  renderer->requiresSwapchainRebuild = 0;
  renderer->swapchain                = ivyCreateVulkanSwapchain(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->mainRenderPass,
      2,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      renderer->colorAttachment.imageView,
      renderer->depthAttachment.imageView,
      &renderer->swapchainImageCount,
      &renderer->frames,
      &renderer->renderSemaphores);
  IVY_ASSERT(renderer->swapchain);
  if (!renderer->swapchain)
    goto error;

  renderer->frameCount = renderer->swapchainImageCount;

  ivyCode = ivyCreateGraphicsProgram(
      &renderer->graphicsContext,
      renderer->mainRenderPass,
      renderer->mainPipelineLayout,
      renderer->swapchainWidth,
      renderer->swapchainHeight,
      "../GLSL/basic.vert.spv",
      "../GLSL/basic.frag.spv",
      IVY_POLYGON_MODE_FILL | IVY_DEPTH_ENABLE | IVY_BLEND_ENABLE |
          IVY_CULL_FRONT | IVY_FRONT_FACE_COUNTERCLOCKWISE,
      &renderer->basicGraphicsProgram);
  IVY_ASSERT(!ivyCode);
  if (ivyCode)
    goto error;

  return IVY_OK;

error:
  return ivyCode;
}

static IvyBool
ivyCheckVulkanResultIfRequiresSwapchainRebuild(VkResult vulkanResult) {
  return VK_SUBOPTIMAL_KHR == vulkanResult ||
         VK_ERROR_OUT_OF_DATE_KHR == vulkanResult;
}

static IvyCode ivyAcquireNextVulkanSwapchainImageIndex(IvyRenderer *renderer) {
  VkResult                     vulkanResult;
  IvyGraphicsRenderSemaphores *semaphores;

  semaphores = ivyGetCurrentGraphicsRenderSemaphores(renderer);

  IVY_ASSERT(renderer->swapchainImageCount);
  IVY_ASSERT(renderer->graphicsContext.device);
  IVY_ASSERT(renderer->swapchain);
  IVY_ASSERT(semaphores->renderDoneSemaphore);
  IVY_ASSERT(semaphores->swapchainImageAvailableSemaphore);

  // FIXME(samuel): check vulkanResult
  vulkanResult = vkAcquireNextImageKHR(
      renderer->graphicsContext.device,
      renderer->swapchain,
      (uint64_t)-1,
      semaphores->swapchainImageAvailableSemaphore,
      VK_NULL_HANDLE,
      &renderer->currentSwapchainImageIndex);

  renderer->requiresSwapchainRebuild =
      ivyCheckVulkanResultIfRequiresSwapchainRebuild(vulkanResult);

  return IVY_OK;
}

IvyCode ivyRequestGraphicsTemporaryBufferFromRenderer(
    IvyRenderer                *renderer,
    uint64_t                    size,
    IvyGraphicsTemporaryBuffer *temporaryBuffer) {
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);
  return ivyRequestGraphicsTemporaryBuffer(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      &frame->temporaryBufferProvider,
      renderer->uniformDescriptorSetLayout,
      size,
      temporaryBuffer);
}

IvyCode ivyBeginGraphicsFrame(IvyRenderer *renderer) {
  IvyCode                      ivyCode;
  VkResult                     vulkanResult;
  VkCommandBufferBeginInfo     commandBufferBeginInfo;
  VkRenderPassBeginInfo        renderPassBeginInfo;
  IvyGraphicsFrame            *frame;
  IvyGraphicsRenderSemaphores *semaphores;

  if (renderer->requiresSwapchainRebuild)
    ivyRebuildGraphicsSwapchain(renderer);

  ivyCode = ivyAcquireNextVulkanSwapchainImageIndex(renderer);
  IVY_ASSERT(!ivyCode);

  frame      = ivyGetCurrentGraphicsFrame(renderer);
  semaphores = ivyGetCurrentGraphicsRenderSemaphores(renderer);

  IVY_ASSERT(frame);
  IVY_ASSERT(frame->inFlightFence);
  vulkanResult = vkWaitForFences(
      renderer->graphicsContext.device,
      1,
      &frame->inFlightFence,
      VK_TRUE,
      (uint64_t)-1);
  IVY_ASSERT(!vulkanResult);

  vulkanResult = vkResetFences(
      renderer->graphicsContext.device,
      1,
      &frame->inFlightFence);
  IVY_ASSERT(!vulkanResult);

  vulkanResult = vkResetCommandPool(
      renderer->graphicsContext.device,
      frame->commandPool,
      0);
  IVY_ASSERT(!vulkanResult);

  ivyClearGraphicsTemporaryBufferProvider(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator,
      &frame->temporaryBufferProvider);

  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  commandBufferBeginInfo.pNext = NULL;
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  commandBufferBeginInfo.pInheritanceInfo = NULL;

  vulkanResult = vkBeginCommandBuffer(
      frame->commandBuffer,
      &commandBufferBeginInfo);
  IVY_ASSERT(!vulkanResult);

  renderPassBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.pNext       = NULL;
  renderPassBeginInfo.renderPass  = renderer->mainRenderPass;
  renderPassBeginInfo.framebuffer = frame->framebuffer;
  renderPassBeginInfo.renderArea.offset.x      = 0;
  renderPassBeginInfo.renderArea.offset.y      = 0;
  renderPassBeginInfo.renderArea.extent.width  = renderer->swapchainWidth;
  renderPassBeginInfo.renderArea.extent.height = renderer->swapchainHeight;
  renderPassBeginInfo.clearValueCount = IVY_ARRAY_LENGTH(renderer->clearValues);
  renderPassBeginInfo.pClearValues    = renderer->clearValues;

  vkCmdBeginRenderPass(
      frame->commandBuffer,
      &renderPassBeginInfo,
      VK_SUBPASS_CONTENTS_INLINE);

  return IVY_OK;
}

IvyCode ivyEndGraphicsFrame(IvyRenderer *renderer) {
  VkResult                     vulkanResult;
  VkSubmitInfo                 submitInfo;
  VkPresentInfoKHR             presentInfo;
  VkPipelineStageFlagBits      stage;
  IvyGraphicsFrame            *frame;
  IvyGraphicsRenderSemaphores *semaphores;

  frame      = ivyGetCurrentGraphicsFrame(renderer);
  semaphores = ivyGetCurrentGraphicsRenderSemaphores(renderer);

  vkCmdEndRenderPass(frame->commandBuffer);
  vulkanResult = vkEndCommandBuffer(frame->commandBuffer);
  IVY_ASSERT(!vulkanResult);

  IVY_ASSERT(semaphores->swapchainImageAvailableSemaphore);
  IVY_ASSERT(semaphores->renderDoneSemaphore);
  IVY_ASSERT(frame->commandBuffer);
  IVY_ASSERT(renderer->graphicsContext.graphicsQueue);

  stage                         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext              = NULL;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = &semaphores->swapchainImageAvailableSemaphore;
  submitInfo.pWaitDstStageMask  = &stage;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &frame->commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = &semaphores->renderDoneSemaphore;

  vulkanResult = vkQueueSubmit(
      renderer->graphicsContext.graphicsQueue,
      1,
      &submitInfo,
      frame->inFlightFence);
  IVY_ASSERT(!vulkanResult);

  presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext              = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = &semaphores->renderDoneSemaphore;
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = &renderer->swapchain;
  presentInfo.pImageIndices      = &renderer->currentSwapchainImageIndex;
  presentInfo.pResults           = NULL;

  // FIXME(samuel): check vulkanResult
  vulkanResult = vkQueuePresentKHR(
      renderer->graphicsContext.presentQueue,
      &presentInfo);

  ++renderer->currentSemaphoreIndex;
  if (renderer->currentSemaphoreIndex == renderer->swapchainImageCount)
    renderer->currentSemaphoreIndex = 0;

  return IVY_OK;
}
