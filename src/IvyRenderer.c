#include "IvyRenderer.h"

#include "IvyGraphicsTexture.h"

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

static VkSwapchainKHR ivyCreateVulkanSwapchain(
    VkPhysicalDevice physicalDevice,
    VkDevice         device,
    VkSurfaceKHR     surface,
    VkRenderPass     mainRenderPass,
    uint32_t         graphicsQueueFamilyIndex,
    uint32_t         presentQueueFamilyIndex,
    uint32_t         minSwapchainImageCount,
    int32_t          width,
    int32_t          height,
    VkFormat         format,
    VkColorSpaceKHR  colorSpace,
    VkPresentModeKHR presentMode,
    VkImageView      colorAttachmentImageView,
    VkImageView      depthAttachmentImageView,
    uint32_t        *swapchainImageCount,
    VkImage        **swapchainImages,
    VkImageView    **swapchainImageViews,
    VkFramebuffer  **swapchainFramebuffers) {
  VkResult                 vulkanResult;
  VkSwapchainKHR           swapchain = VK_NULL_HANDLE;
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  VkSwapchainCreateInfoKHR swapchainCreateInfo;

  uint32_t queueFamilyIndices[2] = {
      graphicsQueueFamilyIndex,
      presentQueueFamilyIndex};

  *swapchainImageCount   = 0;
  *swapchainImages       = NULL;
  *swapchainImageViews   = NULL;
  *swapchainFramebuffers = NULL;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice,
      surface,
      &surfaceCapabilities);

  swapchainCreateInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext   = NULL;
  swapchainCreateInfo.flags   = 0;
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.minImageCount      = minSwapchainImageCount;
  swapchainCreateInfo.imageFormat        = format;
  swapchainCreateInfo.imageColorSpace    = colorSpace;
  swapchainCreateInfo.imageExtent.width  = width;
  swapchainCreateInfo.imageExtent.height = height;
  swapchainCreateInfo.imageArrayLayers   = 1;
  swapchainCreateInfo.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.preTransform       = surfaceCapabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode        = presentMode;
  swapchainCreateInfo.clipped            = VK_TRUE;
  swapchainCreateInfo.oldSwapchain       = VK_NULL_HANDLE;

  if (graphicsQueueFamilyIndex == presentQueueFamilyIndex) {
    swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices   = NULL;
  } else {
    swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
  }

  vulkanResult = vkCreateSwapchainKHR(
      device,
      &swapchainCreateInfo,
      NULL,
      &swapchain);
  if (vulkanResult)
    goto error;

  *swapchainImages = ivyAllocateVulkanSwapchainImages(
      device,
      swapchain,
      swapchainImageCount);
  if (!*swapchainImages)
    goto error;

  *swapchainImageViews = ivyAllocateAndCreateVulkanSwapchainImageViews(
      device,
      format,
      *swapchainImageCount,
      *swapchainImages);
  if (!*swapchainImageViews)
    goto error;

  *swapchainFramebuffers = ivyAllocateAndCreateVulkanSwapchainFramebuffers(
      device,
      *swapchainImageCount,
      *swapchainImageViews,
      width,
      height,
      mainRenderPass,
      colorAttachmentImageView,
      depthAttachmentImageView);
  if (!*swapchainFramebuffers)
    goto error;

  return swapchain;

error:

  if (*swapchainFramebuffers) {
    uint32_t i;
    for (i = 0; i < *swapchainImageCount; ++i)
      vkDestroyFramebuffer(device, (*swapchainFramebuffers)[i], NULL);
    IVY_FREE(*swapchainFramebuffers);
    *swapchainFramebuffers = NULL;
  }

  if (*swapchainImageViews) {
    uint32_t i;
    for (i = 0; i < *swapchainImageCount; ++i)
      vkDestroyImageView(device, (*swapchainImageViews)[i], NULL);
    IVY_FREE(*swapchainImageViews);
    *swapchainImageViews = NULL;
  }

  if (*swapchainImages) {
    IVY_FREE(*swapchainImages);
    *swapchainImages = NULL;
  }

  if (swapchain)
    vkDestroySwapchainKHR(device, swapchain, NULL);

  *swapchainImageCount = 0;
  return VK_NULL_HANDLE;
}

static VkRenderPass ivyCreateMainRenderPass(
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
  depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

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

IvyCode ivyCreateRenderer(IvyApplication *application, IvyRenderer *renderer) {
  IvyCode ivyCode;

  IVY_MEMSET(renderer, 0, sizeof(*renderer));

  ivyCode = ivyCreateGraphicsContext(application, &renderer->graphicsContext);
  if (ivyCode)
    goto error;

  ivyCode = ivyCreateDummyGraphicsMemoryAllocator(
      &renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator);
  if (ivyCode)
    goto error;

  renderer->mainRenderPass = ivyCreateMainRenderPass(
      renderer->graphicsContext.device,
      renderer->graphicsContext.surfaceFormat.format,
      renderer->graphicsContext.depthFormat,
      renderer->graphicsContext.attachmentSampleCounts);
  if (!renderer->mainRenderPass)
    goto error;

  renderer->swapchain = ivyCreateVulkanSwapchain(
      renderer->graphicsContext.physicalDevice,
      renderer->graphicsContext.device,
      renderer->graphicsContext.surface,
      renderer->mainRenderPass,
      renderer->graphicsContext.graphicsQueueFamilyIndex,
      renderer->graphicsContext.presentQueueFamilyIndex,
      2,
      application->lastAddedWindow->framebufferWidth,
      application->lastAddedWindow->framebufferHeight,
      renderer->graphicsContext.surfaceFormat.format,
      renderer->graphicsContext.surfaceFormat.colorSpace,
      renderer->graphicsContext.presentMode,
      renderer->colorAttachment.imageView,
      renderer->depthAttachment.imageView,
      &renderer->swapchainImageCount,
      &renderer->swapchainImages,
      &renderer->swapchainImageViews,
      &renderer->swapchainFramebuffers);
  if (!renderer->swapchain)
    goto error;

  return IVY_OK;

error:
  ivyDestroyRenderer(renderer);
  return ivyCode;
}

void ivyDestroyRenderer(IvyRenderer *renderer) {
  if (renderer->swapchainFramebuffers) {
    uint32_t i;
    for (i = 0; i < renderer->swapchainImageCount; ++i)
      vkDestroyFramebuffer(
          renderer->graphicsContext.device,
          renderer->swapchainFramebuffers[i],
          NULL);
    IVY_FREE(renderer->swapchainFramebuffers);
    renderer->swapchainFramebuffers = NULL;
  }

  if (renderer->swapchainImageViews) {
    uint32_t i;
    for (i = 0; i < renderer->swapchainImageCount; ++i)
      vkDestroyImageView(
          renderer->graphicsContext.device,
          renderer->swapchainImageViews[i],
          NULL);
    IVY_FREE(renderer->swapchainImageViews);
    renderer->swapchainImageViews = NULL;
  }

  if (renderer->swapchainImages) {
    IVY_FREE(renderer->swapchainImages);
    renderer->swapchainImages = NULL;
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
