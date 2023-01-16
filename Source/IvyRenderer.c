#include "IvyRenderer.h"

#include "IvyApplication.h"
#include "IvyGraphicsTexture.h"
#include "IvyLog.h"
#include "IvyVulkanUtilities.h"

#if defined(IVY_ENABLE_VULKAN_VALIDATION_LAYERS)
IVY_INTERNAL char const *const validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"};
#endif

IVY_INTERNAL VkInstanceCreateFlagBits ivyGetVulkanInstanceCreateFlagBits(
    void) {
#ifdef __APPLE__
  return VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
  return 0;
#endif
}

IVY_INTERNAL VkResult ivyCreateVulkanInstance(IvyApplication *application,
    VkInstance *instance) {
  VkApplicationInfo applicationInfo;
  VkInstanceCreateInfo instanceCreateInfo;

  applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  applicationInfo.pNext = NULL;
  applicationInfo.pApplicationName = application->name;
  applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.pEngineName = "No Engine";
  applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = NULL;
  instanceCreateInfo.flags = ivyGetVulkanInstanceCreateFlagBits();
  instanceCreateInfo.pApplicationInfo = &applicationInfo;
#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
  instanceCreateInfo.enabledLayerCount = IVY_ARRAY_LENGTH(validationLayers);
  instanceCreateInfo.ppEnabledLayerNames = validationLayers;
#else  /* IVY_ENABLE_VULKAN_VALIDATION_LAYERS */
  instanceCreateInfo.enabledLayerCount = 0;
  instanceCreateInfo.ppEnabledLayerNames = NULL;
#endif /* IVY_ENABLE_VULKAN_VALIDATION_LAYERS */
  instanceCreateInfo.ppEnabledExtensionNames = ivyGetRequiredVulkanExtensions(
      application, &instanceCreateInfo.enabledExtensionCount);

  if (!instanceCreateInfo.ppEnabledExtensionNames) {
    return VK_ERROR_UNKNOWN;
  }

  return vkCreateInstance(&instanceCreateInfo, NULL, instance);
}

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
#define IVY_VK_INSTANCE_PROC_ADDR(instance, name)                             \
  (PFN_##name) vkGetInstanceProcAddr(instance, #name)
#endif

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
IVY_INTERNAL void ivyEnsureValidationFunctions(VkInstance instance,
    PFN_vkCreateDebugUtilsMessengerEXT *createDebugUtilsMessengerEXT,
    PFN_vkDestroyDebugUtilsMessengerEXT *destroyDebugUtilsMessengerEXT) {
  *createDebugUtilsMessengerEXT =
      IVY_VK_INSTANCE_PROC_ADDR(instance, vkCreateDebugUtilsMessengerEXT);

  *destroyDebugUtilsMessengerEXT =
      IVY_VK_INSTANCE_PROC_ADDR(instance, vkDestroyDebugUtilsMessengerEXT);
}
#endif

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
static VKAPI_ATTR VKAPI_CALL VkBool32 ivyLogVulkanMessages(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    VkDebugUtilsMessengerCallbackDataEXT const *data, void *user) {
  IVY_UNUSED(type);
  IVY_UNUSED(user);
  IVY_UNUSED(data);

  if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    IVY_DEBUG_LOG("[VERBOSE]\n  %s\n", data->pMessage);
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    IVY_DEBUG_LOG("[INFO]\n  %s\n", data->pMessage);
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    IVY_DEBUG_LOG("[WARNING]\n  %s\n", data->pMessage);
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    IVY_DEBUG_LOG("[ERROR]\n  %s\n", data->pMessage);
  }

  return VK_FALSE;
}
#endif

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
IVY_INTERNAL VkResult ivyCreateVulkanDebugMessenger(VkInstance instance,
    PFN_vkCreateDebugUtilsMessengerEXT *createDebugUtilsMessengerEXT,
    PFN_vkDestroyDebugUtilsMessengerEXT *destroyDebugUtilsMessengerEXT,
    VkDebugUtilsMessengerEXT *messenger) {
  VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;

  ivyEnsureValidationFunctions(instance, createDebugUtilsMessengerEXT,
      destroyDebugUtilsMessengerEXT);

  if (!*createDebugUtilsMessengerEXT || !*destroyDebugUtilsMessengerEXT) {
    return VK_ERROR_UNKNOWN;
  }

  debugMessengerCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugMessengerCreateInfo.pNext = NULL;
  debugMessengerCreateInfo.flags = 0;
  debugMessengerCreateInfo.messageSeverity = 0;
  debugMessengerCreateInfo.messageSeverity |=
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
  debugMessengerCreateInfo.messageSeverity |=
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
  debugMessengerCreateInfo.messageSeverity |=
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
  debugMessengerCreateInfo.messageSeverity |=
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugMessengerCreateInfo.messageType = 0;
  debugMessengerCreateInfo.messageType |=
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  debugMessengerCreateInfo.messageType |=
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debugMessengerCreateInfo.messageType |=
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugMessengerCreateInfo.pfnUserCallback = ivyLogVulkanMessages;
  debugMessengerCreateInfo.pUserData = NULL;

  return (*createDebugUtilsMessengerEXT)(instance, &debugMessengerCreateInfo,
      NULL, messenger);
}
#endif /* IVY_ENABLE_VULKAN_VALIDATION_LAYERS */

IVY_INTERNAL VkQueueFamilyProperties *ivyAllocateVulkanQueueFamilyProperties(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice device,
    uint32_t *count) {
  VkQueueFamilyProperties *properties;

  vkGetPhysicalDeviceQueueFamilyProperties(device, count, NULL);
  properties = ivyAllocateMemory(allocator, *count * sizeof(*properties));
  if (!properties) {
    return NULL;
  }
  vkGetPhysicalDeviceQueueFamilyProperties(device, count, properties);
  return properties;
}

IVY_INTERNAL IvyBool ivyAreVulkanQueueFamilyIndicesValid(
    uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex) {
  return (uint32_t)-1 != graphicsQueueFamilyIndex &&
         (uint32_t)-1 != presentQueueFamilyIndex;
}

IVY_INTERNAL void ivyFindVulkanQueueFamilyIndices(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice device,
    VkSurfaceKHR surface, uint32_t *selectedGraphicsQueueFamilyIndex,
    uint32_t *selectedPresentQueueFamilyIndex) {
  uint32_t index;
  uint32_t queueFamilyPropertiesCount;
  VkQueueFamilyProperties *queueFamilyProperties;

  *selectedGraphicsQueueFamilyIndex = (uint32_t)-1;
  *selectedPresentQueueFamilyIndex = (uint32_t)-1;

  queueFamilyProperties = ivyAllocateVulkanQueueFamilyProperties(allocator,
      device, &queueFamilyPropertiesCount);
  if (!queueFamilyProperties) {
    return;
  }

  for (index = 0; index < queueFamilyPropertiesCount; ++index) {
    VkBool32 isSurfaceSupported;

    if (queueFamilyProperties[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      *selectedGraphicsQueueFamilyIndex = index;
    }

    vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface,
        &isSurfaceSupported);
    if (isSurfaceSupported) {
      *selectedPresentQueueFamilyIndex = index;
    }

    if (ivyAreVulkanQueueFamilyIndicesValid(*selectedGraphicsQueueFamilyIndex,
            *selectedPresentQueueFamilyIndex)) {
      goto cleanup;
    }
  }

cleanup:
  ivyFreeMemory(allocator, queueFamilyProperties);
}

IVY_INTERNAL IvyCode ivyFindAvailableVulkanPhysicalDevices(
    IvyAnyMemoryAllocator allocator, VkInstance instance,
    uint32_t *physicalDeviceCount, VkPhysicalDevice **physicalDevices) {
  VkResult vulkanResult;

  vulkanResult =
      vkEnumeratePhysicalDevices(instance, physicalDeviceCount, NULL);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    return ivyVulkanResultAsIvyCode(vulkanResult);
  }

  *physicalDevices = ivyAllocateMemory(allocator,
      *physicalDeviceCount * sizeof(**physicalDevices));
  if (!*physicalDevices) {
    *physicalDeviceCount = 0;
    return IVY_ERROR_NO_MEMORY;
  }

  vulkanResult = vkEnumeratePhysicalDevices(instance, physicalDeviceCount,
      *physicalDevices);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    *physicalDeviceCount = 0;
    return ivyVulkanResultAsIvyCode(vulkanResult);
  }

  return IVY_OK;
}

IVY_INTERNAL VkExtensionProperties *ivyAllocateVulkanExtensionsProperties(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice physicalDevice,
    uint32_t *count) {
  VkResult vulkanResult;
  VkExtensionProperties *extensions;
  vulkanResult =
      vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, count, NULL);
  if (vulkanResult) {
    return NULL;
  }
  extensions = ivyAllocateMemory(allocator, *count * sizeof(*extensions));
  if (!extensions) {
    return NULL;
  }
  vulkanResult = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL,
      count, extensions);
  if (vulkanResult) {
    ivyFreeMemory(allocator, extensions);
    return NULL;
  }
  return extensions;
}

IVY_INTERNAL IvyBool ivyDoAllVulkanRequiredExtensionsExist(
    uint32_t availableExtensionCount,
    VkExtensionProperties *availableExtensions,
    uint32_t requiredExtensionCount, char const *const *requiredExtensions) {
  uint32_t requiredExtensionIndex;
  for (requiredExtensionIndex = 0;
       requiredExtensionIndex < requiredExtensionCount;
       ++requiredExtensionIndex) {
    IvyBool existsInAvailableExtensions = 0;
    uint32_t availableExtensionIndex;
    for (availableExtensionIndex = 0;
         availableExtensionIndex < availableExtensionCount &&
         !existsInAvailableExtensions;
         ++availableExtensionIndex) {
      if (!IVY_STRNCMP(requiredExtensions[requiredExtensionIndex],
              availableExtensions[availableExtensionIndex].extensionName,
              VK_MAX_EXTENSION_NAME_SIZE)) {
        existsInAvailableExtensions = 1;
      }
    }

    if (!existsInAvailableExtensions) {
      return 0;
    }
  }

  return 1;
}

IVY_INTERNAL IvyBool ivyDoesVulkanPhysicalDeviceSupportRequiredExtensions(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice physicalDevice,
    uint32_t requiredExtensionCount, char const *const *requiredExtensions) {
  IvyBool exist;
  uint32_t availableExtensionCount;
  VkExtensionProperties *availableExtensions;

  availableExtensions = ivyAllocateVulkanExtensionsProperties(allocator,
      physicalDevice, &availableExtensionCount);
  if (!availableExtensions || !availableExtensionCount) {
    return 0;
  }

  exist = ivyDoAllVulkanRequiredExtensionsExist(availableExtensionCount,
      availableExtensions, requiredExtensionCount, requiredExtensions);

  ivyFreeMemory(allocator, availableExtensions);

  return exist;
}

#define IVY_TEST_VULKAN_DEPTH_FORMAT(device, format)                          \
  {                                                                           \
    VkFormatProperties formatProperties;                                      \
    vkGetPhysicalDeviceFormatProperties(device, format, &formatProperties);   \
    if (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT &                      \
        formatProperties.optimalTilingFeatures)                               \
      return format;                                                          \
  }
IVY_INTERNAL VkFormat ivySelectVulkanDepthFormat(VkPhysicalDevice device) {
  IVY_TEST_VULKAN_DEPTH_FORMAT(device, VK_FORMAT_D24_UNORM_S8_UINT);
  IVY_TEST_VULKAN_DEPTH_FORMAT(device, VK_FORMAT_D32_SFLOAT_S8_UINT);
  return VK_FORMAT_UNDEFINED;
}
#undef IVY_TEST_VULKAN_DEPTH_FORMAT

IVY_INTERNAL VkSurfaceFormatKHR *ivyAllocateVulkanSurfaceFormats(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice device,
    VkSurfaceKHR surface, uint32_t *count) {
  VkResult vulkanResult;
  VkSurfaceFormatKHR *formats;
  vulkanResult =
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, NULL);
  if (vulkanResult) {
    return NULL;
  }
  formats = ivyAllocateMemory(allocator, *count * sizeof(*formats));
  if (!formats) {
    return NULL;
  }
  vulkanResult =
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, formats);
  if (vulkanResult) {
    ivyFreeMemory(allocator, formats);
    return NULL;
  }
  return formats;
}

IVY_INTERNAL IvyBool ivyDoesVulkanFormatExists(uint32_t surfaceFormatCount,
    VkSurfaceFormatKHR *surfaceFormats, VkFormat format,
    VkColorSpaceKHR colorSpace) {
  uint32_t index;

  for (index = 0; index < surfaceFormatCount; ++index) {
    if (format != surfaceFormats[index].format) {
      continue;
    }

    if (colorSpace != surfaceFormats[index].colorSpace) {
      continue;
    }

    return 1;
  }

  return 0;
}

IVY_INTERNAL IvyBool ivyDoesVulkanPhysicalDeviceSupportFormat(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice device,
    VkSurfaceKHR surface, VkFormat format, VkColorSpaceKHR colorSpace) {
  IvyBool exist;
  uint32_t surfaceFormatCount;
  VkSurfaceFormatKHR *surfaceFormats;

  surfaceFormats = ivyAllocateVulkanSurfaceFormats(allocator, device, surface,
      &surfaceFormatCount);
  if (!surfaceFormats || !surfaceFormatCount) {
    return 0;
  }

  exist = ivyDoesVulkanFormatExists(surfaceFormatCount, surfaceFormats, format,
      colorSpace);

  ivyFreeMemory(allocator, surfaceFormats);
  return exist;
}

IVY_INTERNAL IvyBool ivyDoesVulkanPresentModeExist(uint32_t presentModeCount,
    VkPresentModeKHR const *presentModes,
    VkPresentModeKHR requiredPresentMode) {
  uint32_t index;
  for (index = 0; index < presentModeCount; ++index) {
    if (presentModes[index] == requiredPresentMode) {
      return 1;
    }
  }

  return 0;
}

IVY_INTERNAL VkPresentModeKHR *ivyAllocateVulkanPresentModes(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice device,
    VkSurfaceKHR surface, uint32_t *count) {
  VkResult vulkanResult;
  VkPresentModeKHR *presentModes;
  vulkanResult =
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count, NULL);
  if (vulkanResult) {
    return NULL;
  }
  presentModes = ivyAllocateMemory(allocator, *count * sizeof(*presentModes));
  if (!presentModes) {
    return NULL;
  }
  vulkanResult = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
      count, presentModes);
  if (vulkanResult) {
    ivyFreeMemory(allocator, presentModes);
    return NULL;
  }
  return presentModes;
}

IVY_INTERNAL IvyBool ivyDoesVulkanPhysicalDeviceSupportPresentMode(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice device,
    VkSurfaceKHR surface, VkPresentModeKHR requiredPresentMode) {
  IvyBool exist;
  uint32_t presentModeCount;
  VkPresentModeKHR *presentModes;

  presentModes = ivyAllocateVulkanPresentModes(allocator, device, surface,
      &presentModeCount);
  if (!presentModes || !presentModeCount) {
    return 0;
  }

  exist = ivyDoesVulkanPresentModeExist(presentModeCount, presentModes,
      requiredPresentMode);

  ivyFreeMemory(allocator, presentModes);
  return exist;
}

IVY_INTERNAL IvyBool ivyDoesVulkanPhysicalDeviceSupportSampleCount(
    VkPhysicalDevice device, VkSampleCountFlagBits samples) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);
  return properties.limits.framebufferColorSampleCounts & samples &&
         properties.limits.framebufferDepthSampleCounts & samples;
}

IVY_INTERNAL char const *const requiredVulkanExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
#if __APPLE__
    ,
    "VK_KHR_portability_subset"
// , "VK_KHR_get_physical_device_properties2"
#endif
};

IVY_INTERNAL VkPhysicalDevice ivySelectVulkanPhysicalDevice(
    IvyAnyMemoryAllocator allocator, VkSurfaceKHR surface,
    uint32_t availablePhysicalDeviceCount,
    VkPhysicalDevice *availablePhysicalDevices, VkFormat requestedFormat,
    VkColorSpaceKHR requestedColorSpace, VkPresentModeKHR requestedPresentMode,
    VkSampleCountFlagBits requestedSampleCount,
    uint32_t *selectedGraphicsQueueFamilyIndex,
    uint32_t *selectedPresentQueueFamilyIndex, VkFormat *selectedDepthFormat) {
  uint32_t index;
  for (index = 0; index < availablePhysicalDeviceCount; ++index) {
    VkPhysicalDevice device = availablePhysicalDevices[index];

    if (!ivyDoesVulkanPhysicalDeviceSupportRequiredExtensions(allocator,
            device, IVY_ARRAY_LENGTH(requiredVulkanExtensions),
            requiredVulkanExtensions)) {
      continue;
    }

    ivyFindVulkanQueueFamilyIndices(allocator, device, surface,
        selectedGraphicsQueueFamilyIndex, selectedPresentQueueFamilyIndex);
    if (!ivyAreVulkanQueueFamilyIndicesValid(*selectedGraphicsQueueFamilyIndex,
            *selectedPresentQueueFamilyIndex)) {
      continue;
    }

    *selectedDepthFormat = ivySelectVulkanDepthFormat(device);
    if (VK_FORMAT_UNDEFINED == *selectedDepthFormat) {
      continue;
    }

    if (!ivyDoesVulkanPhysicalDeviceSupportFormat(allocator, device, surface,
            requestedFormat, requestedColorSpace)) {
      continue;
    }

    if (!ivyDoesVulkanPhysicalDeviceSupportPresentMode(allocator, device,
            surface, requestedPresentMode)) {
      continue;
    }

    if (!ivyDoesVulkanPhysicalDeviceSupportSampleCount(device,
            requestedSampleCount)) {
      continue;
    }

    return device;
  }

  return VK_NULL_HANDLE;
}

IVY_INTERNAL VkResult ivyCreateVulkanDevice(IvyAnyMemoryAllocator allocator,
    VkSurfaceKHR surface, uint32_t availablePhysicalDeviceCount,
    VkPhysicalDevice *availablePhysicalDevices, VkFormat requiredFormat,
    VkColorSpaceKHR requiredColorSpace, VkPresentModeKHR requiredPresentMode,
    VkSampleCountFlagBits requiredSampleCount,
    VkPhysicalDevice *selectedPhysicalDevice, VkFormat *selectedDepthFormat,
    uint32_t *selectedGraphicsQueueFamilyIndex,
    uint32_t *selectedPresentQueueFamilyIndex, VkQueue *createdGraphicsQueue,
    VkQueue *createdPresentQueue, VkDevice *device) {
  float const queuePriority = 1.0F;
  VkResult vulkanResult;
  VkPhysicalDeviceFeatures physicalDeviceFeatures;
  VkDeviceQueueCreateInfo queueCreateInfos[2];
  VkDeviceCreateInfo deviceCreateInfo;

  *selectedPhysicalDevice = ivySelectVulkanPhysicalDevice(allocator, surface,
      availablePhysicalDeviceCount, availablePhysicalDevices, requiredFormat,
      requiredColorSpace, requiredPresentMode, requiredSampleCount,
      selectedGraphicsQueueFamilyIndex, selectedPresentQueueFamilyIndex,
      selectedDepthFormat);
  IVY_ASSERT(*selectedPhysicalDevice);
  if (!*selectedPhysicalDevice) {
    return VK_ERROR_UNKNOWN;
  }

  queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfos[0].pNext = NULL;
  queueCreateInfos[0].flags = 0;
  queueCreateInfos[0].queueFamilyIndex = *selectedGraphicsQueueFamilyIndex;
  queueCreateInfos[0].queueCount = 1;
  queueCreateInfos[0].pQueuePriorities = &queuePriority;

  queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfos[1].pNext = NULL;
  queueCreateInfos[1].flags = 0;
  queueCreateInfos[1].queueFamilyIndex = *selectedPresentQueueFamilyIndex;
  queueCreateInfos[1].queueCount = 1;
  queueCreateInfos[1].pQueuePriorities = &queuePriority;

  vkGetPhysicalDeviceFeatures(*selectedPhysicalDevice,
      &physicalDeviceFeatures);

  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = NULL;
  deviceCreateInfo.flags = 0;
  if (*selectedGraphicsQueueFamilyIndex == *selectedPresentQueueFamilyIndex) {
    deviceCreateInfo.queueCreateInfoCount = 1;
  } else {
    deviceCreateInfo.queueCreateInfoCount = 2;
  }
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
  deviceCreateInfo.enabledLayerCount = 0;      /* deprecated */
  deviceCreateInfo.ppEnabledLayerNames = NULL; /* deprecated */
  deviceCreateInfo.enabledExtensionCount =
      IVY_ARRAY_LENGTH(requiredVulkanExtensions);
  deviceCreateInfo.ppEnabledExtensionNames = requiredVulkanExtensions;
  deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

  vulkanResult =
      vkCreateDevice(*selectedPhysicalDevice, &deviceCreateInfo, NULL, device);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    return vulkanResult;
  }

  vkGetDeviceQueue(*device, *selectedGraphicsQueueFamilyIndex, 0,
      createdGraphicsQueue);
  vkGetDeviceQueue(*device, *selectedPresentQueueFamilyIndex, 0,
      createdPresentQueue);

  return vulkanResult;
}

IVY_INTERNAL VkResult ivyCreateVulkanTransientCommandPool(VkDevice device,
    uint32_t family, VkCommandPool *commandPool) {
  VkCommandPoolCreateInfo commandPoolCreateInfo;

  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = NULL;
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  commandPoolCreateInfo.queueFamilyIndex = family;

  return vkCreateCommandPool(device, &commandPoolCreateInfo, NULL,
      commandPool);
}

#define IVY_MAX_DESCRIPTOR_POOL_TYPES 7
#define IVY_DEFAULT_DESCRIPTOR_COUNT 256

IVY_INTERNAL VkResult ivyCreateVulkanGlobalDescriptorPool(VkDevice device,
    VkDescriptorPool *descriptorPool) {
  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
  VkDescriptorPoolSize descriptorPoolSizes[IVY_MAX_DESCRIPTOR_POOL_TYPES];

  descriptorPoolSizes[0].descriptorCount = IVY_DEFAULT_DESCRIPTOR_COUNT;
  descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

  descriptorPoolSizes[1].descriptorCount = IVY_DEFAULT_DESCRIPTOR_COUNT;
  descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;

  descriptorPoolSizes[2].descriptorCount = IVY_DEFAULT_DESCRIPTOR_COUNT;
  descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

  descriptorPoolSizes[3].descriptorCount = IVY_DEFAULT_DESCRIPTOR_COUNT;
  descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

  descriptorPoolSizes[4].descriptorCount = IVY_DEFAULT_DESCRIPTOR_COUNT;
  descriptorPoolSizes[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  descriptorPoolSizes[5].descriptorCount = IVY_DEFAULT_DESCRIPTOR_COUNT;
  descriptorPoolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

  descriptorPoolSizes[6].descriptorCount = IVY_DEFAULT_DESCRIPTOR_COUNT;
  descriptorPoolSizes[6].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

  descriptorPoolCreateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolCreateInfo.pNext = NULL;
  descriptorPoolCreateInfo.flags =
      VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  descriptorPoolCreateInfo.maxSets =
      IVY_DEFAULT_DESCRIPTOR_COUNT * IVY_ARRAY_LENGTH(descriptorPoolSizes);
  descriptorPoolCreateInfo.poolSizeCount =
      IVY_ARRAY_LENGTH(descriptorPoolSizes);
  descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;

  return vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL,
      descriptorPool);
}

IVY_INTERNAL VkResult ivyCreateVulkanSwapchainFramebuffer(VkDevice device,
    int32_t width, int32_t height, VkRenderPass mainRenderPass,
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

  return vkCreateFramebuffer(device, &framebufferCreateInfo, NULL,
      framebuffer);
}

IVY_INTERNAL VkResult ivyCreateVulkanCommandPool(VkDevice device,
    uint32_t graphicsQueueFamilyIndex, VkCommandPoolCreateFlagBits flags,
    VkCommandPool *commandPool) {
  VkCommandPoolCreateInfo commandPoolCreateInfo;

  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = NULL;
  commandPoolCreateInfo.flags = flags;
  commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

  return vkCreateCommandPool(device, &commandPoolCreateInfo, NULL,
      commandPool);
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

IVY_INTERNAL VkImage *ivyAllocateVulkanSwapchainImages(
    IvyAnyMemoryAllocator allocator, VkDevice device, VkSwapchainKHR swapchain,
    uint32_t *swapchainImageCount) {
  VkResult vulkanResult;
  VkImage *images;

  vulkanResult =
      vkGetSwapchainImagesKHR(device, swapchain, swapchainImageCount, NULL);
  if (vulkanResult) {
    return NULL;
  }

  images =
      ivyAllocateMemory(allocator, *swapchainImageCount * sizeof(*images));
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

IVY_INTERNAL VkResult ivyCreateVulkanSwapchain(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface, VkFormat surfaceFormat,
    VkColorSpaceKHR surfaceColorSpace, VkPresentModeKHR presentMode,
    VkDevice device, uint32_t graphicsQueueFamilyIndex,
    uint32_t presentQueueFamilyIndex, uint32_t minSwapchainImageCount,
    int32_t width, int32_t height, VkSwapchainKHR *swapchain) {
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  VkSwapchainCreateInfoKHR swapchainCreateInfo;
  uint32_t queueFamilyIndices[2];

  queueFamilyIndices[0] = graphicsQueueFamilyIndex;
  queueFamilyIndices[1] = presentQueueFamilyIndex;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
      &surfaceCapabilities);

  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext = NULL;
  swapchainCreateInfo.flags = 0;
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.minImageCount = minSwapchainImageCount;
  swapchainCreateInfo.imageFormat = surfaceFormat;
  swapchainCreateInfo.imageColorSpace = surfaceColorSpace;
  swapchainCreateInfo.imageExtent.width = width;
  swapchainCreateInfo.imageExtent.height = height;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = presentMode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  if (graphicsQueueFamilyIndex == presentQueueFamilyIndex) {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = NULL;
  } else {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  }

  return vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, swapchain);
}

IVY_INTERNAL void ivyDestroyGraphicsFrames(IvyAnyMemoryAllocator allocator,
    IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    VkDescriptorPool descriptorPool, uint32_t frameCount,
    IvyGraphicsFrame *frames) {
  uint32_t frameIndex;

  if (!frames) {
    return;
  }

  for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
    IvyGraphicsFrame *frame = &frames[frameIndex];

    if (frame->garbageChunks) {
      uint32_t garbageChunkIndex;
      for (garbageChunkIndex = 0; garbageChunkIndex < frame->garbageChunkCount;
           ++garbageChunkIndex) {
        IvyGraphicsRenderBufferChunk *chunk =
            &frame->garbageChunks[garbageChunkIndex];

        ivyFreeGraphicsMemory(device, graphicsMemoryAllocator, &chunk->memory);

        if (chunk->descriptorSet) {
          vkFreeDescriptorSets(device->logicalDevice, descriptorPool, 1,
              &chunk->descriptorSet);
          chunk->descriptorSet = VK_NULL_HANDLE;
        }

        if (chunk->buffer) {
          vkDestroyBuffer(device->logicalDevice, chunk->buffer, NULL);
          chunk->buffer = VK_NULL_HANDLE;
        }
      }

      ivyFreeMemory(allocator, frame->garbageChunks);
    }

    ivyFreeGraphicsMemory(device, graphicsMemoryAllocator,
        &frame->currentChunk.memory);

    if (frame->currentChunk.descriptorSet) {
      vkFreeDescriptorSets(device->logicalDevice, descriptorPool, 1,
          &frame->currentChunk.descriptorSet);
      frame->currentChunk.descriptorSet = VK_NULL_HANDLE;
    }

    if (frame->currentChunk.buffer) {
      vkDestroyBuffer(device->logicalDevice, frame->currentChunk.buffer, NULL);
      frame->currentChunk.buffer = VK_NULL_HANDLE;
    }

    if (frame->inFlightFence) {
      vkDestroyFence(device->logicalDevice, frame->inFlightFence, NULL);
      frame->inFlightFence = NULL;
    }

    if (frame->framebuffer) {
      vkDestroyFramebuffer(device->logicalDevice, frame->framebuffer, NULL);
      frame->framebuffer = VK_NULL_HANDLE;
    }

    if (frame->imageView) {
      vkDestroyImageView(device->logicalDevice, frame->imageView, NULL);
      frame->imageView = VK_NULL_HANDLE;
    }

    if (frame->commandBuffer) {
      vkFreeCommandBuffers(device->logicalDevice, frame->commandPool, 1,
          &frame->commandBuffer);
      frame->commandBuffer = VK_NULL_HANDLE;
    }

    if (frame->commandPool) {
      vkDestroyCommandPool(device->logicalDevice, frame->commandPool, NULL);
      frame->commandPool = VK_NULL_HANDLE;
    }
  }

  ivyFreeMemory(allocator, frames);
}

IVY_INTERNAL IvyCode ivyCreateGraphicsFrames(IvyAnyMemoryAllocator allocator,
    IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    VkDescriptorPool descriptorPool, VkRenderPass mainRenderPass,
    uint32_t swapchainImageCount, VkImage *swapchainImages,
    VkFormat surfaceFormat, int32_t width, int32_t height,
    VkImageView colorAttachmentImageView, VkImageView depthAttachmentImageView,
    IvyGraphicsFrame **frames) {
  IvyCode ivyCode;
  uint32_t frameIndex;
  IvyGraphicsFrame *currentFrames;

  currentFrames = ivyAllocateMemory(allocator,
      swapchainImageCount * sizeof(*currentFrames));
  if (!currentFrames) {
    return IVY_ERROR_NO_MEMORY;
  }

  IVY_MEMSET(currentFrames, 0, sizeof(*currentFrames));

  for (frameIndex = 0; frameIndex < swapchainImageCount; ++frameIndex) {
    VkResult vulkanResult;
    IvyGraphicsFrame *frame = &currentFrames[frameIndex];

    vulkanResult = ivyCreateVulkanCommandPool(device->logicalDevice,
        device->graphicsQueueFamilyIndex, 0, &frame->commandPool);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }

    vulkanResult = ivyAllocateVulkanCommandBuffer(device->logicalDevice,
        frame->commandPool, &frame->commandBuffer);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }

    vulkanResult = ivyCreateVulkanImageView(device->logicalDevice,
        swapchainImages[frameIndex], VK_IMAGE_ASPECT_COLOR_BIT, surfaceFormat,
        &frame->imageView);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }

    vulkanResult =
        ivyCreateVulkanSwapchainFramebuffer(device->logicalDevice, width,
            height, mainRenderPass, frame->imageView, colorAttachmentImageView,
            depthAttachmentImageView, &frame->framebuffer);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }

    vulkanResult = ivyCreateVulkanSignaledFence(device->logicalDevice,
        &frame->inFlightFence);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }

    frame->currentChunk.buffer = VK_NULL_HANDLE;
    frame->currentChunk.descriptorSet = VK_NULL_HANDLE;
    frame->currentChunk.memory.memory = VK_NULL_HANDLE;

    frame->garbageChunkCount = 0;
    frame->garbageChunks = NULL;
  }

  *frames = currentFrames;

  return IVY_OK;

error:
  ivyDestroyGraphicsFrames(allocator, device, graphicsMemoryAllocator,
      descriptorPool, swapchainImageCount, currentFrames);
  *frames = NULL;
  return ivyCode;
}

IVY_INTERNAL void ivyDestroyGraphicsRenderSemaphores(
    IvyAnyMemoryAllocator allocator, IvyGraphicsDevice *device, uint32_t count,
    IvyGraphicsRenderSemaphores *semaphores) {
  uint32_t index;

  if (!semaphores) {
    return;
  }

  for (index = 0; index < count; ++index) {
    IvyGraphicsRenderSemaphores *semaphore = &semaphores[index];
    if (semaphore->swapchainImageAvailableSemaphore) {
      vkDestroySemaphore(device->logicalDevice,
          semaphore->swapchainImageAvailableSemaphore, NULL);
      semaphore->swapchainImageAvailableSemaphore = VK_NULL_HANDLE;
    }

    if (semaphore->renderDoneSemaphore) {
      vkDestroySemaphore(device->logicalDevice, semaphore->renderDoneSemaphore,
          NULL);
      semaphore->renderDoneSemaphore = VK_NULL_HANDLE;
    }
  }

  ivyFreeMemory(allocator, semaphores);
}

IVY_INTERNAL IvyCode ivyCreateGraphicsRenderSemaphores(
    IvyAnyMemoryAllocator allocator, IvyGraphicsDevice *device, uint32_t count,
    IvyGraphicsRenderSemaphores **semaphores) {
  IvyCode ivyCode;
  uint32_t index;
  IvyGraphicsRenderSemaphores *currentSemaphores;

  currentSemaphores =
      ivyAllocateMemory(allocator, count * sizeof(*currentSemaphores));
  if (!currentSemaphores) {
    return IVY_ERROR_NO_MEMORY;
  }

  for (index = 0; index < count; ++index) {
    VkResult vulkanResult;
    IvyGraphicsRenderSemaphores *semaphore = &currentSemaphores[index];
    vulkanResult = ivyCreateVulkanSemaphore(device->logicalDevice,
        &semaphore->renderDoneSemaphore);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }

    vulkanResult = ivyCreateVulkanSemaphore(device->logicalDevice,
        &semaphore->swapchainImageAvailableSemaphore);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
      goto error;
    }
  }

  *semaphores = currentSemaphores;

  return IVY_OK;

error:
  ivyDestroyGraphicsRenderSemaphores(allocator, device, count,
      currentSemaphores);
  *semaphores = NULL;
  return ivyCode;
}

IVY_INTERNAL VkResult ivyCreateVulkanMainRenderPass(VkDevice device,
    VkFormat colorFormat, VkFormat depthFormat,
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
  VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;

  descriptorSetLayoutBinding.binding = 0;
  descriptorSetLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorSetLayoutBinding.descriptorCount = 1;
  descriptorSetLayoutBinding.stageFlags = 0;
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

IVY_INTERNAL VkResult ivyCreateVulkanMainPipelineLayout(VkDevice device,
    VkDescriptorSetLayout uniformDescriptorSetLayout,
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

IVY_INTERNAL void ivyDestroyGraphicsAttachment(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    IvyGraphicsAttachment *attachment) {
  ivyFreeGraphicsMemory(device, graphicsMemoryAllocator, &attachment->memory);

  if (attachment->imageView) {
    vkDestroyImageView(device->logicalDevice, attachment->imageView, NULL);
    attachment->imageView = VK_NULL_HANDLE;
  }

  if (attachment->image) {
    vkDestroyImage(device->logicalDevice, attachment->image, NULL);
    attachment->image = VK_NULL_HANDLE;
  }
}

IVY_INTERNAL IvyCode ivyCreateGraphicsAttachment(IvyGraphicsDevice *device,
    IvyAnyGraphicsMemoryAllocator graphicsMemoryAllocator,
    VkImageUsageFlagBits usage, VkSampleCountFlagBits sampleCount,
    VkFormat format, VkImageAspectFlagBits aspect, int32_t width,
    int32_t height, IvyGraphicsAttachment *attachment) {
  VkResult vulkanResult;
  IvyCode ivyCode = IVY_OK;

  IVY_MEMSET(attachment, 0, sizeof(*attachment));

  attachment->width = width;
  attachment->height = height;

  vulkanResult = ivyCreateVulkanImage(device->logicalDevice, width, height, 1,
      sampleCount, usage, format, &attachment->image);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode =
      ivyAllocateAndBindGraphicsMemoryToImage(device, graphicsMemoryAllocator,
          IVY_GPU_LOCAL, attachment->image, &attachment->memory);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  vulkanResult = ivyCreateVulkanImageView(device->logicalDevice,
      attachment->image, aspect, format, &attachment->imageView);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  return IVY_OK;

error:
  ivyDestroyGraphicsAttachment(device, graphicsMemoryAllocator, attachment);
  return ivyCode;
}

IVY_API IvyCode ivyCreateRenderer(IvyAnyMemoryAllocator allocator,
    IvyApplication *application, IvyRenderer **renderer) {
  IvyCode ivyCode = IVY_OK;
  VkResult vulkanResult;
  VkImage *swapchainImages = NULL;
  IvyRenderer *currentRenderer;

  currentRenderer = ivyAllocateMemory(allocator, sizeof(*currentRenderer));
  IVY_ASSERT(currentRenderer);
  if (!currentRenderer) {
    ivyCode = IVY_ERROR_NO_MEMORY;
    goto error;
  }

  IVY_MEMSET(currentRenderer, 0, sizeof(*currentRenderer));

  currentRenderer->application = application;
  currentRenderer->ownerMemoryAllocator = allocator;

  vulkanResult =
      ivyCreateVulkanInstance(application, &currentRenderer->instance);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanDebugMessenger(currentRenderer->instance,
      &currentRenderer->createDebugUtilsMessengerEXT,
      &currentRenderer->destroyDebugUtilsMessengerEXT,
      &currentRenderer->debugMessenger);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanSurface(currentRenderer->instance, application,
      &currentRenderer->surface);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyFindAvailableVulkanPhysicalDevices(allocator,
      currentRenderer->instance,
      &currentRenderer->availablePhysicalDeviceCount,
      &currentRenderer->availablePhysicalDevices);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  currentRenderer->surfaceFormat = VK_FORMAT_B8G8R8A8_SRGB;
  currentRenderer->surfaceColorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  currentRenderer->presentMode = VK_PRESENT_MODE_FIFO_KHR;
  currentRenderer->attachmentsSampleCounts = VK_SAMPLE_COUNT_2_BIT;

  vulkanResult = ivyCreateVulkanDevice(allocator, currentRenderer->surface,
      currentRenderer->availablePhysicalDeviceCount,
      currentRenderer->availablePhysicalDevices,
      currentRenderer->surfaceFormat, currentRenderer->surfaceColorspace,
      currentRenderer->presentMode, currentRenderer->attachmentsSampleCounts,
      &currentRenderer->device.physicalDevice, &currentRenderer->depthFormat,
      &currentRenderer->device.graphicsQueueFamilyIndex,
      &currentRenderer->device.presentQueueFamilyIndex,
      &currentRenderer->device.graphicsQueue,
      &currentRenderer->device.presentQueue,
      &currentRenderer->device.logicalDevice);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanTransientCommandPool(
      currentRenderer->device.logicalDevice,
      currentRenderer->device.graphicsQueueFamilyIndex,
      &currentRenderer->transientCommandPool);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanGlobalDescriptorPool(
      currentRenderer->device.logicalDevice,
      &currentRenderer->globalDescriptorPool);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  ivyCode = ivyCreateDummyGraphicsMemoryAllocator(&currentRenderer->device,
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
      currentRenderer->device.logicalDevice, currentRenderer->surfaceFormat,
      currentRenderer->depthFormat, currentRenderer->attachmentsSampleCounts,
      &currentRenderer->mainRenderPass);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanUniformDescriptorSetLayout(
      currentRenderer->device.logicalDevice,
      &currentRenderer->uniformDescriptorSetLayout);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult = ivyCreateVulkanTextureDescriptorSetLayout(
      currentRenderer->device.logicalDevice,
      &currentRenderer->textureDescriptorSetLayout);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  vulkanResult =
      ivyCreateVulkanMainPipelineLayout(currentRenderer->device.logicalDevice,
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

  ivyCode = ivyCreateGraphicsAttachment(&currentRenderer->device,
      &currentRenderer->defaultGraphicsMemoryAllocator,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      currentRenderer->attachmentsSampleCounts, currentRenderer->surfaceFormat,
      VK_IMAGE_ASPECT_COLOR_BIT, currentRenderer->swapchainWidth,
      currentRenderer->swapchainHeight, &currentRenderer->colorAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  ivyCode = ivyCreateGraphicsAttachment(&currentRenderer->device,
      &currentRenderer->defaultGraphicsMemoryAllocator,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      currentRenderer->attachmentsSampleCounts, currentRenderer->depthFormat,
      VK_IMAGE_ASPECT_DEPTH_BIT, currentRenderer->swapchainWidth,
      currentRenderer->swapchainHeight, &currentRenderer->depthAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  currentRenderer->requiresSwapchainRebuild = 0;

  vulkanResult = ivyCreateVulkanSwapchain(
      currentRenderer->device.physicalDevice, currentRenderer->surface,
      currentRenderer->surfaceFormat, currentRenderer->surfaceColorspace,
      currentRenderer->presentMode, currentRenderer->device.logicalDevice,
      currentRenderer->device.graphicsQueueFamilyIndex,
      currentRenderer->device.presentQueueFamilyIndex, 2,
      currentRenderer->swapchainWidth, currentRenderer->swapchainHeight,
      &currentRenderer->swapchain);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  swapchainImages = ivyAllocateVulkanSwapchainImages(allocator,
      currentRenderer->device.logicalDevice, currentRenderer->swapchain,
      &currentRenderer->swapchainImageCount);
  IVY_ASSERT(swapchainImages);
  if (swapchainImages) {
    ivyCode = ivyCreateGraphicsFrames(allocator, &currentRenderer->device,
        &currentRenderer->defaultGraphicsMemoryAllocator,
        currentRenderer->globalDescriptorPool, currentRenderer->mainRenderPass,
        currentRenderer->swapchainImageCount, swapchainImages,
        currentRenderer->surfaceFormat, currentRenderer->swapchainWidth,
        currentRenderer->swapchainHeight,
        currentRenderer->colorAttachment.imageView,
        currentRenderer->depthAttachment.imageView, &currentRenderer->frames);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }
    ivyFreeMemory(allocator, swapchainImages);
  } else {
    ivyCode = IVY_ERROR_NO_MEMORY;
    goto error;
  }

  ivyCode = ivyCreateGraphicsRenderSemaphores(allocator,
      &currentRenderer->device, currentRenderer->swapchainImageCount,
      &currentRenderer->renderSemaphores);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  ivyCode = ivyCreateGraphicsProgram(allocator, &currentRenderer->device,
      currentRenderer->attachmentsSampleCounts,
      currentRenderer->mainRenderPass, currentRenderer->mainPipelineLayout,
      currentRenderer->swapchainWidth, currentRenderer->swapchainHeight,
      "../GLSL/Basic.vert.spv", "../GLSL/Basic.frag.spv",
      IVY_VERTEX_332_ENABLE | IVY_POLYGON_MODE_FILL | IVY_DEPTH_ENABLE |
          IVY_BLEND_ENABLE | IVY_CULL_BACK | IVY_FRONT_FACE_COUNTERCLOCKWISE,
      &currentRenderer->basicGraphicsProgram);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  *renderer = currentRenderer;

  return IVY_OK;

error:
  ivyDestroyRenderer(allocator, currentRenderer);

  if (swapchainImages) {
    ivyFreeMemory(allocator, swapchainImages);
  }

  *renderer = NULL;
  return ivyCode;
}

IVY_API void ivyDestroyRenderer(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer) {
  if (!renderer) {
    return;
  }

  ivyDestroyGraphicsProgram(&renderer->device,
      &renderer->basicGraphicsProgram);

  ivyDestroyGraphicsRenderSemaphores(allocator, &renderer->device,
      renderer->swapchainImageCount, renderer->renderSemaphores);
  renderer->renderSemaphores = NULL;

  ivyDestroyGraphicsFrames(allocator, &renderer->device,
      &renderer->defaultGraphicsMemoryAllocator,
      renderer->globalDescriptorPool, renderer->swapchainImageCount,
      renderer->frames);
  renderer->frames = NULL;

  if (renderer->swapchain) {
    vkDestroySwapchainKHR(renderer->device.logicalDevice, renderer->swapchain,
        NULL);
    renderer->swapchain = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsAttachment(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, &renderer->depthAttachment);
  ivyDestroyGraphicsAttachment(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, &renderer->colorAttachment);

  if (renderer->mainPipelineLayout) {
    vkDestroyPipelineLayout(renderer->device.logicalDevice,
        renderer->mainPipelineLayout, NULL);
    renderer->mainPipelineLayout = VK_NULL_HANDLE;
  }

  if (renderer->textureDescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(renderer->device.logicalDevice,
        renderer->textureDescriptorSetLayout, NULL);
    renderer->textureDescriptorSetLayout = VK_NULL_HANDLE;
  }

  if (renderer->uniformDescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(renderer->device.logicalDevice,
        renderer->uniformDescriptorSetLayout, NULL);
    renderer->uniformDescriptorSetLayout = VK_NULL_HANDLE;
  }

  if (renderer->mainRenderPass) {
    vkDestroyRenderPass(renderer->device.logicalDevice,
        renderer->mainRenderPass, NULL);
    renderer->mainRenderPass = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsMemoryAllocator(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator);

  if (renderer->globalDescriptorPool) {
    vkDestroyDescriptorPool(renderer->device.logicalDevice,
        renderer->globalDescriptorPool, NULL);
    renderer->globalDescriptorPool = VK_NULL_HANDLE;
  }

  if (renderer->transientCommandPool) {
    vkDestroyCommandPool(renderer->device.logicalDevice,
        renderer->transientCommandPool, NULL);
    renderer->transientCommandPool = VK_NULL_HANDLE;
  }

  if (renderer->device.logicalDevice) {
    vkDestroyDevice(renderer->device.logicalDevice, NULL);
    renderer->device.logicalDevice = VK_NULL_HANDLE;
  }

  ivyFreeMemory(allocator, renderer->availablePhysicalDevices);
  renderer->availablePhysicalDevices = NULL;

  if (renderer->surface) {
    vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
    renderer->surface = VK_NULL_HANDLE;
  }

  if (renderer->debugMessenger) {
    renderer->destroyDebugUtilsMessengerEXT(renderer->instance,
        renderer->debugMessenger, NULL);
    renderer->debugMessenger = VK_NULL_HANDLE;
  }

  if (renderer->instance) {
    vkDestroyInstance(renderer->instance, NULL);
    renderer->instance = VK_NULL_HANDLE;
  }

  ivyFreeMemory(allocator, renderer);
}

IVY_API IvyGraphicsFrame *ivyGetCurrentGraphicsFrame(IvyRenderer *renderer) {
  return &renderer->frames[renderer->currentSwapchainImageIndex];
}

IVY_INTERNAL IvyGraphicsRenderSemaphores *
ivyGetCurrentGraphicsRenderSemaphores(IvyRenderer *renderer) {
  return &renderer->renderSemaphores[renderer->currentSemaphoreIndex];
}

IVY_INTERNAL void ivyDestroyGraphicsResourcesForSwapchainRebuild(
    IvyRenderer *renderer) {
  IvyAnyMemoryAllocator allocator = renderer->ownerMemoryAllocator;

  if (renderer->device.logicalDevice) {
    vkDeviceWaitIdle(renderer->device.logicalDevice);
  }

  ivyDestroyGraphicsProgram(&renderer->device,
      &renderer->basicGraphicsProgram);

  if (renderer->renderSemaphores) {
    ivyDestroyGraphicsRenderSemaphores(allocator, &renderer->device,
        renderer->swapchainImageCount, renderer->renderSemaphores);
    renderer->renderSemaphores = NULL;
  }

  if (renderer->frames) {
    ivyDestroyGraphicsFrames(allocator, &renderer->device,
        &renderer->defaultGraphicsMemoryAllocator,
        renderer->globalDescriptorPool, renderer->swapchainImageCount,
        renderer->frames);
    renderer->frames = NULL;
  }

  if (renderer->swapchain) {
    vkDestroySwapchainKHR(renderer->device.logicalDevice, renderer->swapchain,
        NULL);
    renderer->swapchain = VK_NULL_HANDLE;
  }

  ivyDestroyGraphicsAttachment(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, &renderer->depthAttachment);

  ivyDestroyGraphicsAttachment(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator, &renderer->colorAttachment);
}

IVY_API IvyCode ivyRebuildGraphicsSwapchain(IvyRenderer *renderer) {
  // FIXME(samuel): create first destroy second
  VkResult vulkanResult;
  IvyCode ivyCode;
  IvyAnyMemoryAllocator allocator = renderer->ownerMemoryAllocator;
  IvyApplication *application = renderer->application;
  VkImage *swapchainImages = NULL;

  ivyDestroyGraphicsResourcesForSwapchainRebuild(renderer);

  renderer->requiresSwapchainRebuild = 0;
  renderer->currentSemaphoreIndex = 0;
  renderer->currentSwapchainImageIndex = 0;
  renderer->swapchainWidth = application->lastAddedWindow->framebufferWidth;
  renderer->swapchainHeight = application->lastAddedWindow->framebufferHeight;

  ivyCode = ivyCreateGraphicsAttachment(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, renderer->attachmentsSampleCounts,
      renderer->surfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT,
      renderer->swapchainWidth, renderer->swapchainHeight,
      &renderer->colorAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  ivyCode = ivyCreateGraphicsAttachment(&renderer->device,
      &renderer->defaultGraphicsMemoryAllocator,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      renderer->attachmentsSampleCounts, renderer->depthFormat,
      VK_IMAGE_ASPECT_DEPTH_BIT, renderer->swapchainWidth,
      renderer->swapchainHeight, &renderer->depthAttachment);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  vulkanResult = ivyCreateVulkanSwapchain(renderer->device.physicalDevice,
      renderer->surface, renderer->surfaceFormat, renderer->surfaceColorspace,
      renderer->presentMode, renderer->device.logicalDevice,
      renderer->device.graphicsQueueFamilyIndex,
      renderer->device.presentQueueFamilyIndex, 2, renderer->swapchainWidth,
      renderer->swapchainHeight, &renderer->swapchain);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    ivyCode = ivyVulkanResultAsIvyCode(vulkanResult);
    goto error;
  }

  swapchainImages = ivyAllocateVulkanSwapchainImages(allocator,
      renderer->device.logicalDevice, renderer->swapchain,
      &renderer->swapchainImageCount);
  IVY_ASSERT(swapchainImages);
  if (swapchainImages) {
    ivyCode = ivyCreateGraphicsFrames(allocator, &renderer->device,
        &renderer->defaultGraphicsMemoryAllocator,
        renderer->globalDescriptorPool, renderer->mainRenderPass,
        renderer->swapchainImageCount, swapchainImages,
        renderer->surfaceFormat, renderer->swapchainWidth,
        renderer->swapchainHeight, renderer->colorAttachment.imageView,
        renderer->depthAttachment.imageView, &renderer->frames);
    if (ivyCode) {
      goto error;
    }
    ivyFreeMemory(allocator, swapchainImages);
  } else {
    ivyCode = IVY_ERROR_NO_MEMORY;
    goto error;
  }

  ivyCode = ivyCreateGraphicsRenderSemaphores(allocator, &renderer->device,
      renderer->swapchainImageCount, &renderer->renderSemaphores);
  if (ivyCode) {
    goto error;
  }

  ivyCode = ivyCreateGraphicsProgram(allocator, &renderer->device,
      renderer->attachmentsSampleCounts, renderer->mainRenderPass,
      renderer->mainPipelineLayout, renderer->swapchainWidth,
      renderer->swapchainHeight, "../GLSL/Basic.vert.spv",
      "../GLSL/Basic.frag.spv",
      IVY_VERTEX_332_ENABLE | IVY_POLYGON_MODE_FILL | IVY_DEPTH_ENABLE |
          IVY_BLEND_ENABLE | IVY_CULL_BACK | IVY_FRONT_FACE_COUNTERCLOCKWISE,
      &renderer->basicGraphicsProgram);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    goto error;
  }

  return IVY_OK;

error:
  // FIXME(samuel): try to build before destroying
  ivyDestroyGraphicsResourcesForSwapchainRebuild(renderer);

  if (swapchainImages) {
    ivyFreeMemory(allocator, swapchainImages);
  }

  return ivyCode;
}

IVY_INTERNAL IvyBool ivyCheckIfVulkanSwapchainRequiresRebuild(
    VkResult vulkanResult) {
  return VK_SUBOPTIMAL_KHR == vulkanResult ||
         VK_ERROR_OUT_OF_DATE_KHR == vulkanResult;
}

IVY_INTERNAL IvyCode ivyAcquireNextVulkanSwapchainImageIndex(
    IvyRenderer *renderer) {
  VkResult vulkanResult;
  IvyGraphicsRenderSemaphores *semaphores;

  semaphores = ivyGetCurrentGraphicsRenderSemaphores(renderer);

  // FIXME(samuel): check vulkanResult
  vulkanResult = vkAcquireNextImageKHR(renderer->device.logicalDevice,
      renderer->swapchain, (uint64_t)-1,
      semaphores->swapchainImageAvailableSemaphore, VK_NULL_HANDLE,
      &renderer->currentSwapchainImageIndex);

  if (ivyCheckIfVulkanSwapchainRequiresRebuild(vulkanResult)) {
    renderer->requiresSwapchainRebuild = 1;
    return IVY_OK;
  } else if (!vulkanResult) {
    return IVY_OK;
  } else {
    return ivyVulkanResultAsIvyCode(vulkanResult);
  }
}

#define ivyAlignTo(value, to) (((value) + (to)-1) & ~((to)-1))

IVY_INTERNAL uint64_t ivyAlignTo256(uint64_t value) {
  return ivyAlignTo(value, 256);
}

IVY_INTERNAL void ivyWriteVulkanUniformDynamicDescriptorSet(VkDevice device,
    VkBuffer buffer, VkDescriptorSet descriptorSet, uint64_t uniformSize) {
  VkDescriptorBufferInfo descriptorBufferInfo;
  VkWriteDescriptorSet writeDescriptorSet;

  descriptorBufferInfo.buffer = buffer;
  descriptorBufferInfo.offset = 0;
  descriptorBufferInfo.range = uniformSize;

  writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSet.pNext = NULL;
  writeDescriptorSet.dstSet = descriptorSet;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.dstArrayElement = 0;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.descriptorType =
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  writeDescriptorSet.pImageInfo = NULL;
  writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
  writeDescriptorSet.pTexelBufferView = NULL;

  vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

IVY_API IvyCode ivyRequestGraphicsTemporaryBuffer(IvyRenderer *renderer,
    uint64_t size, IvyGraphicsTemporaryBuffer *temporaryBuffer) {
  IvyAnyGraphicsMemoryAllocator allocator = renderer->ownerMemoryAllocator;
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);
  IvyGraphicsRenderBufferChunk *currentChunk = &frame->currentChunk;
  uint64_t const offset = currentChunk->offset;
  uint64_t const requiredSize = offset + size;

  if (!frame->currentChunk.buffer || frame->currentChunk.size < requiredSize) {
    VkResult vulkanResult;
    IvyCode ivyCode;
    uint64_t newSize;
    VkBuffer newBuffer = VK_NULL_HANDLE;
    VkDescriptorSet newDescriptorSet = VK_NULL_HANDLE;
    IvyGraphicsMemory newMemory;

    newSize = ivyAlignTo256(requiredSize * 2);

    vulkanResult = ivyCreateVulkanBuffer(renderer->device.logicalDevice,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        newSize, &newBuffer);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      return ivyVulkanResultAsIvyCode(vulkanResult);
    }

    ivyCode = ivyAllocateAndBindGraphicsMemoryToBuffer(&renderer->device,
        &renderer->defaultGraphicsMemoryAllocator, IVY_CPU_VISIBLE, newBuffer,
        &newMemory);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      vkDestroyBuffer(renderer->device.logicalDevice, newBuffer, NULL);
      return ivyCode;
    }

    vulkanResult = ivyAllocateVulkanDescriptorSet(
        renderer->device.logicalDevice, renderer->globalDescriptorPool,
        renderer->uniformDescriptorSetLayout, &newDescriptorSet);
    IVY_ASSERT(!vulkanResult);
    if (vulkanResult) {
      ivyFreeGraphicsMemory(&renderer->device,
          &renderer->defaultGraphicsMemoryAllocator, &newMemory);
      vkDestroyBuffer(renderer->device.logicalDevice, newBuffer, NULL);
      return ivyVulkanResultAsIvyCode(vulkanResult);
    }

    if (currentChunk->buffer) {
      IvyGraphicsRenderBufferChunk *newGarbageChunks;
      uint32_t newGarbageChunkCount = frame->garbageChunkCount + 1;

      newGarbageChunks = ivyReallocateMemory(allocator, frame->garbageChunks,
          newGarbageChunkCount * sizeof(*newGarbageChunks));
      if (!newGarbageChunks) {
        ivyFreeGraphicsMemory(&renderer->device,
            &renderer->defaultGraphicsMemoryAllocator, &newMemory);
        vkDestroyBuffer(renderer->device.logicalDevice, newBuffer, NULL);
        vkFreeDescriptorSets(renderer->device.logicalDevice,
            renderer->globalDescriptorPool, 1, &newDescriptorSet);
        return IVY_ERROR_NO_MEMORY;
      }

      IVY_MEMCPY(&newGarbageChunks[frame->garbageChunkCount], currentChunk,
          sizeof(*currentChunk));

      frame->garbageChunks = newGarbageChunks;
      frame->garbageChunkCount = newGarbageChunkCount;
    }

    ivyWriteVulkanUniformDynamicDescriptorSet(renderer->device.logicalDevice,
        newBuffer, newDescriptorSet, sizeof(IvyGraphicsProgramUniform));

    currentChunk->size = newSize;
    currentChunk->offset = 0;
    currentChunk->buffer = newBuffer;
    currentChunk->descriptorSet = newDescriptorSet;
    IVY_MEMCPY(&currentChunk->memory, &newMemory, sizeof(newMemory));
  }

  temporaryBuffer->data = ((uint8_t *)currentChunk->memory.data) + offset;
  temporaryBuffer->size = size;
  temporaryBuffer->offsetInU64 = offset;
  temporaryBuffer->offsetInU32 = offset;
  temporaryBuffer->buffer = currentChunk->buffer;
  temporaryBuffer->descriptorSet = currentChunk->descriptorSet;

  currentChunk->offset = ivyAlignTo256(offset + size);

  return IVY_OK;
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
  vulkanResult = vkWaitForFences(renderer->device.logicalDevice, 1,
      &frame->inFlightFence, VK_TRUE, (uint64_t)-1);
  IVY_ASSERT(!vulkanResult);

  vulkanResult =
      vkResetFences(renderer->device.logicalDevice, 1, &frame->inFlightFence);
  IVY_ASSERT(!vulkanResult);

  vulkanResult = vkResetCommandPool(renderer->device.logicalDevice,
      frame->commandPool, 0);
  IVY_ASSERT(!vulkanResult);

  frame->currentChunk.offset = 0;

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
  renderPassBeginInfo.clearValueCount =
      IVY_ARRAY_LENGTH(renderer->clearValues);
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
  IVY_ASSERT(renderer->device.graphicsQueue);

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

  vulkanResult = vkQueueSubmit(renderer->device.graphicsQueue, 1, &submitInfo,
      frame->inFlightFence);
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
      vkQueuePresentKHR(renderer->device.presentQueue, &presentInfo);

  ++renderer->currentSemaphoreIndex;
  if (renderer->currentSemaphoreIndex == renderer->swapchainImageCount) {
    renderer->currentSemaphoreIndex = 0;
  }

  renderer->boundGraphicsProgram = NULL;

  return IVY_OK;
}

IVY_API void ivyBindGraphicsProgram(IvyRenderer *renderer,
    IvyGraphicsProgram *program) {
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  if (renderer->boundGraphicsProgram == program) {
    return;
  }
  renderer->boundGraphicsProgram = program;
  vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      program->pipeline);
}
