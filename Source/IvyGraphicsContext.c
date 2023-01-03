#include "IvyGraphicsContext.h"

#include "IvyLog.h"
#include "IvyMemoryAllocator.h"

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

IVY_INTERNAL VkInstance ivyCreateVulkanInstance(IvyApplication *application) {
  VkResult vulkanResult;
  VkInstance instance;
  VkApplicationInfo applicationInfo;
  VkInstanceCreateInfo instanceCreateInfo;

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
#endif /* IVY_ENABLE_VULKAN_VALIDATION_LAYERSOWL_ENABLE_VALIDATION */
  instanceCreateInfo.ppEnabledExtensionNames = ivyGetRequiredVulkanExtensions(
      application, &instanceCreateInfo.enabledExtensionCount);

  if (!instanceCreateInfo.ppEnabledExtensionNames) {
    return VK_NULL_HANDLE;
  }

  vulkanResult = vkCreateInstance(&instanceCreateInfo, NULL, &instance);
  if (vulkanResult) {
    IVY_DEBUG_LOG("%i\n", vulkanResult);
    return VK_NULL_HANDLE;
  }

  return instance;
}

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
// FIXME(samuel): per context function pointers
PFN_vkCreateDebugUtilsMessengerEXT ivyCreateDebugUtilsMessengerEXT = NULL;
PFN_vkDestroyDebugUtilsMessengerEXT ivyDestroyDebugUtilsMessengerEXT = NULL;
#endif

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
#define IVY_VK_INSTANCE_PROC_ADDR(instance, name)                             \
  (PFN_##name) vkGetInstanceProcAddr(instance, #name)
#endif

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
IVY_INTERNAL void ivyEnsureValidationFunctions(VkInstance instance) {
  if (!ivyCreateDebugUtilsMessengerEXT) {
    ivyCreateDebugUtilsMessengerEXT =
        IVY_VK_INSTANCE_PROC_ADDR(instance, vkCreateDebugUtilsMessengerEXT);
  }

  if (!ivyDestroyDebugUtilsMessengerEXT) {
    ivyDestroyDebugUtilsMessengerEXT =
        IVY_VK_INSTANCE_PROC_ADDR(instance, vkDestroyDebugUtilsMessengerEXT);
  }
}
#endif

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
#include <stdio.h>
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
IVY_INTERNAL VkDebugUtilsMessengerEXT ivyCreateVulkanDebugMessenger(
    VkInstance instance) {
  VkResult vulkanResult;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;

  ivyEnsureValidationFunctions(instance);

  if (!ivyCreateDebugUtilsMessengerEXT || !ivyDestroyDebugUtilsMessengerEXT) {
    return VK_NULL_HANDLE;
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

  vulkanResult = ivyCreateDebugUtilsMessengerEXT(instance,
      &debugMessengerCreateInfo, NULL, &debugMessenger);
  if (vulkanResult) {
    return VK_NULL_HANDLE;
  }

  return debugMessenger;
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

IVY_INTERNAL void ivyFindAvailableVulkanPhysicalDevices(VkInstance instance,
    uint32_t *physicalDeviceCount, VkPhysicalDevice *physicalDevices) {
  VkResult vulkanResult;

  vulkanResult =
      vkEnumeratePhysicalDevices(instance, physicalDeviceCount, NULL);
  if (vulkanResult) {
    return;
  }

  if (IVY_MAX_AVAILABLE_DEVICES <= *physicalDeviceCount) {
    *physicalDeviceCount = 0;
    return;
  }

  vulkanResult = vkEnumeratePhysicalDevices(instance, physicalDeviceCount,
      physicalDevices);
  if (vulkanResult) {
    *physicalDeviceCount = 0;
    return;
  }
}

IVY_INTERNAL VkExtensionProperties *ivyAllocateVulkanExtensionsProperties(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice physicalDevice,
    uint32_t *count) {
  VkExtensionProperties *extensions;
  vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, count, NULL);
  extensions = ivyAllocateMemory(allocator, *count * sizeof(*extensions));
  if (!extensions) {
    return NULL;
  }
  vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, count,
      extensions);
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

IVY_INTERNAL int ivyDoesVulkanPhysicalDeviceSupportRequiredExtensions(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice physicalDevice,
    uint32_t requiredExtensionCount, char const *const *requiredExtensions) {
  int exist;
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
  VkSurfaceFormatKHR *formats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, NULL);
  formats = ivyAllocateMemory(allocator, *count * sizeof(*formats));
  if (!formats) {
    return NULL;
  }
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, formats);
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
  int exist;
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
  VkPresentModeKHR *presentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count, NULL);
  presentModes = ivyAllocateMemory(allocator, *count * sizeof(*presentModes));
  if (!presentModes) {
    return NULL;
  }
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count,
      presentModes);
  return presentModes;
}

IVY_INTERNAL IvyBool ivyDoesVulkanPhysicalDeviceSupportPresentMode(
    IvyAnyMemoryAllocator allocator, VkPhysicalDevice device,
    VkSurfaceKHR surface, VkPresentModeKHR requiredPresentMode) {
  int exists;
  uint32_t presentModeCount;
  VkPresentModeKHR *presentModes;

  presentModes = ivyAllocateVulkanPresentModes(allocator, device, surface,
      &presentModeCount);
  if (!presentModes || !presentModeCount) {
    return 0;
  }

  exists = ivyDoesVulkanPresentModeExist(presentModeCount, presentModes,
      requiredPresentMode);

  ivyFreeMemory(allocator, presentModes);
  return exists;
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

IVY_INTERNAL VkDevice ivyCreateVulkanDevice(IvyAnyMemoryAllocator allocator,
    VkSurfaceKHR surface, uint32_t availablePhysicalDeviceCount,
    VkPhysicalDevice *availablePhysicalDevices, VkFormat requiredFormat,
    VkColorSpaceKHR requiredColorSpace, VkPresentModeKHR requiredPresentMode,
    VkSampleCountFlagBits requiredSampleCount,
    VkPhysicalDevice *selectedPhysicalDevice, VkFormat *selectedDepthFormat,
    uint32_t *selectedGraphicsQueueFamilyIndex,
    uint32_t *selectedPresentQueueFamilyIndex, VkQueue *createdGraphicsQueue,
    VkQueue *createdPresentQueue) {
  float const queuePriority = 1.0F;
  VkResult vulkanResult;
  VkDevice device;
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
    return VK_NULL_HANDLE;
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

  vulkanResult = vkCreateDevice(*selectedPhysicalDevice, &deviceCreateInfo,
      NULL, &device);
  IVY_ASSERT(!vulkanResult);
  if (vulkanResult) {
    return VK_NULL_HANDLE;
  }

  vkGetDeviceQueue(device, *selectedGraphicsQueueFamilyIndex, 0,
      createdGraphicsQueue);
  vkGetDeviceQueue(device, *selectedPresentQueueFamilyIndex, 0,
      createdPresentQueue);

  return device;
}

VkCommandPool ivyCreateVulkanTransientCommandPool(VkDevice device,
    uint32_t family) {
  VkResult vulkanResult;
  VkCommandPool commandPool;
  VkCommandPoolCreateInfo commandPoolCreateInfo;

  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = NULL;
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  commandPoolCreateInfo.queueFamilyIndex = family;

  vulkanResult =
      vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);
  if (vulkanResult) {
    return VK_NULL_HANDLE;
  }

  return commandPool;
}

IVY_API void ivyDestroyGraphicsContext(IvyAnyMemoryAllocator allocator,
    IvyGraphicsContext *context) {
  if (context->globalDescriptorPool) {
    vkDestroyDescriptorPool(context->device, context->globalDescriptorPool,
        NULL);
    context->globalDescriptorPool = VK_NULL_HANDLE;
  }

  if (context->transientCommandPool) {
    vkDestroyCommandPool(context->device, context->transientCommandPool, NULL);
    context->transientCommandPool = VK_NULL_HANDLE;
  }

  if (context->device) {
    vkDestroyDevice(context->device, NULL);
    context->device = VK_NULL_HANDLE;
  }

  if (context->surface) {
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    context->surface = VK_NULL_HANDLE;
  }

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
  if (context->debugMessenger) {
    ivyDestroyDebugUtilsMessengerEXT(context->instance,
        context->debugMessenger, NULL);
    context->debugMessenger = VK_NULL_HANDLE;
  }
#endif

  if (context->instance) {
    vkDestroyInstance(context->instance, NULL);
    context->instance = VK_NULL_HANDLE;
  }

  ivyFreeMemory(allocator, context);
}

#define IVY_MAX_DESCRIPTOR_POOL_TYPES 7
#define IVY_DEFAULT_DESCRIPTOR_COUNT 256

VkDescriptorPool ivyCreateVulkanGlobalDescriptorPool(VkDevice device) {
  VkResult vulkanResult;
  VkDescriptorPool descriptorPool;
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

  vulkanResult = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo,
      NULL, &descriptorPool);
  if (vulkanResult) {
    return VK_NULL_HANDLE;
  }

  return descriptorPool;
}

IVY_API IvyGraphicsContext *ivyCreateGraphicsContext(
    IvyAnyMemoryAllocator allocator, IvyApplication *application) {
  IvyGraphicsContext *context;

  context = ivyAllocateMemory(allocator, sizeof(*context));
  if (!context) {
    return NULL;
  }

  IVY_MEMSET(context, 0, sizeof(*context));

  context->application = application;

  context->instance = ivyCreateVulkanInstance(application);
  IVY_ASSERT(context->instance);
  if (!context->instance) {
    goto error;
  }

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
  context->debugMessenger = ivyCreateVulkanDebugMessenger(context->instance);
  IVY_ASSERT(context->debugMessenger);
  if (!context->debugMessenger) {
    goto error;
  }
#else
  context->debugMessenger = VK_NULL_HANDLE;
#endif

  context->surface =
      ivyCreateVulkanSurface(context->instance, context->application);
  IVY_ASSERT(context->surface);
  if (!context->surface) {
    goto error;
  }

  ivyFindAvailableVulkanPhysicalDevices(context->instance,
      &context->availableDeviceCount, context->availableDevices);
  IVY_ASSERT(context->availableDeviceCount);
  if (!context->availableDeviceCount) {
    goto error;
  }

  context->surfaceFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
  context->surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  context->presentMode = VK_PRESENT_MODE_FIFO_KHR;
  context->attachmentSampleCounts = VK_SAMPLE_COUNT_2_BIT;

  context->device = ivyCreateVulkanDevice(allocator, context->surface,
      context->availableDeviceCount, context->availableDevices,
      context->surfaceFormat.format, context->surfaceFormat.colorSpace,
      context->presentMode, context->attachmentSampleCounts,
      &context->physicalDevice, &context->depthFormat,
      &context->graphicsQueueFamilyIndex, &context->presentQueueFamilyIndex,
      &context->graphicsQueue, &context->presentQueue);
  IVY_ASSERT(context->device);
  if (!context->device) {
    goto error;
  }

  context->transientCommandPool = ivyCreateVulkanTransientCommandPool(
      context->device, context->graphicsQueueFamilyIndex);
  IVY_ASSERT(context->transientCommandPool);
  if (!context->transientCommandPool) {
    goto error;
  }

  context->globalDescriptorPool =
      ivyCreateVulkanGlobalDescriptorPool(context->device);
  IVY_ASSERT(context->globalDescriptorPool);
  if (!context->globalDescriptorPool) {
    goto error;
  }

  return context;

error:
  ivyDestroyGraphicsContext(allocator, context);
  return NULL;
}

IVY_API VkCommandBuffer ivyAllocateVulkanCommandBuffer(VkDevice device,
    VkCommandPool commandPool) {
  VkResult vulkanResult;
  VkCommandBuffer commandBuffer;
  VkCommandBufferAllocateInfo commandBufferAllocateInfo;

  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.pNext = NULL;
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = 1;

  vulkanResult = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
      &commandBuffer);
  if (vulkanResult) {
    return VK_NULL_HANDLE;
  }

  return commandBuffer;
}

IVY_API VkCommandBuffer ivyAllocateOneTimeCommandBuffer(
    IvyGraphicsContext *context) {
  VkResult vulkanResult;
  VkCommandBuffer commandBuffer;
  VkCommandBufferBeginInfo beginInfo;

  if (!context) {
    return VK_NULL_HANDLE;
  }

  commandBuffer = ivyAllocateVulkanCommandBuffer(context->device,
      context->transientCommandPool);
  if (!commandBuffer) {
    goto error;
  }

  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = NULL;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = NULL;

  vulkanResult = vkBeginCommandBuffer(commandBuffer, &beginInfo);
  if (vulkanResult) {
    goto error;
  }

  return commandBuffer;

error:
  ivyFreeOneTimeCommandBuffer(context, commandBuffer);
  return VK_NULL_HANDLE;
}

IVY_API void ivyFreeOneTimeCommandBuffer(IvyGraphicsContext *context,
    VkCommandBuffer commandBuffer) {
  if (!context || !commandBuffer) {
    return;
  }

  vkFreeCommandBuffers(context->device, context->transientCommandPool, 1,
      &commandBuffer);
}

IVY_API IvyCode ivySubmitOneTimeCommandBuffer(IvyGraphicsContext *context,
    VkCommandBuffer commandBuffer) {
  VkResult vulkanResult;

  VkSubmitInfo submitInfo;

  if (!context || !commandBuffer) {
    return IVY_INVALID_VALUE;
  }

  vkEndCommandBuffer(commandBuffer);

  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = NULL;
  submitInfo.pWaitDstStageMask = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = NULL;
  vulkanResult =
      vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  if (vulkanResult) {
    return IVY_NO_GRAPHICS_MEMORY;
  }

  vulkanResult = vkQueueWaitIdle(context->graphicsQueue);
  if (vulkanResult) {
    return IVY_NO_GRAPHICS_MEMORY;
  }

  return IVY_OK;
}
