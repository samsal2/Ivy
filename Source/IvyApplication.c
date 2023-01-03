#include "IvyApplication.h"

#include "IvyGraphicsContext.h"

/* clang-format off */
#define GLFW_INCLUDE_NONE
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
/* clang-format on */

IVY_INTERNAL int doesAnApplicationAlreadyExist = 0;

#define IVY_IS_VALID_WINDOW(w) (!!(w)->opaque)

IVY_INTERNAL void ivyWindowSizeCallback(GLFWwindow *opaque, int width,
    int height) {
  IvyWindow *window = glfwGetWindowUserPointer(opaque);
  window->windowWidth = width;
  window->windowHeight = height;
  window->resized = 1;
}

IVY_INTERNAL void ivyFramebufferSizeCallback(GLFWwindow *opaque, int width,
    int height) {
  IvyWindow *window = glfwGetWindowUserPointer(opaque);
  window->framebufferWidth = width;
  window->framebufferHeight = height;
}

IVY_INTERNAL void ivyInvalidateWindow(IvyWindow *window) {
  IVY_ASSERT(window);

  window->opaque = NULL;
  window->windowWidth = 0;
  window->windowHeight = 0;
  window->framebufferWidth = 0;
  window->framebufferHeight = 0;
}

IvyApplication *ivyCreateApplication(IvyAnyMemoryAllocator allocator) {
  int32_t index;
  IvyApplication *application;

  IVY_ASSERT(allocator);

  if (doesAnApplicationAlreadyExist) {
    return NULL;
  }

  application = ivyAllocateMemory(allocator, sizeof(*application));
  if (!application) {
    return NULL;
  }

  application->opaque = NULL;
  application->lastAddedWindow = NULL;

  if (!glfwInit()) {
    ivyFreeMemory(allocator, application);
    return NULL;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    ivyInvalidateWindow(&application->windows[index]);
  }

  doesAnApplicationAlreadyExist = 1;

  return application;
}

IVY_API void ivyDestroyApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication *application) {
  ivyPollApplicationEvents(application);
  glfwTerminate();
  ivyFreeMemory(allocator, application);
  doesAnApplicationAlreadyExist = 0;
}

IVY_INTERNAL IvyWindow *ivyNextInvalidWindow(IvyApplication *application) {
  int32_t index;

  IVY_ASSERT(application);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    IvyWindow *window = &application->windows[index];

    if (!IVY_IS_VALID_WINDOW(window)) {
      return window;
    }
  }

  return NULL;
}

IVY_API IvyWindow *ivyAddWindow(IvyApplication *application, int32_t width,
    int32_t height, char const *title) {
  IvyWindow *window;

  IVY_ASSERT(application);
  IVY_ASSERT(title);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  if (!application) {
    return NULL;
  }

  window = ivyNextInvalidWindow(application);
  if (!window) {
    return NULL;
  }

  window->opaque =
      glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
  if (!window->opaque) {
    return NULL;
  }

  glfwSetWindowUserPointer(window->opaque, window);
  glfwSetFramebufferSizeCallback(window->opaque, ivyFramebufferSizeCallback);
  glfwSetWindowSizeCallback(window->opaque, ivyWindowSizeCallback);

  window->resized = 0;
  glfwGetWindowSize(window->opaque, &window->windowWidth,
      &window->windowHeight);
  glfwGetFramebufferSize(window->opaque, &window->framebufferWidth,
      &window->framebufferHeight);

  return application->lastAddedWindow = window;
}

IVY_INTERNAL IvyBool ivyIsWindowInApplication(IvyApplication *application,
    IvyWindow *window) {
  int32_t index;

  IVY_ASSERT(application);
  IVY_ASSERT(window);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    if (&application->windows[index] == window) {
      return 1;
    }
  }

  return 0;
}

IVY_API IvyWindow *ivyFirstValidWindow(IvyApplication *application) {
  int32_t index;

  IVY_ASSERT(application);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    IvyWindow *window = &application->windows[index];

    if (IVY_IS_VALID_WINDOW(window)) {
      return window;
    }
  }

  return NULL;
}

IVY_API IvyCode ivyDestroyWindow(IvyApplication *application,
    IvyWindow *window) {
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  if (!application || !window || !IVY_IS_VALID_WINDOW(window)) {
    return IVY_INVALID_VALUE;
  }

  if (!ivyIsWindowInApplication(application, window)) {
    return IVY_INVALID_VALUE;
  }

  glfwDestroyWindow(window->opaque);
  window->opaque = NULL;

  // FIXME(samuel): use a FIFO or something
  application->lastAddedWindow = ivyFirstValidWindow(application);

  return IVY_OK;
}

IVY_API IvyBool ivyShouldApplicationClose(IvyApplication *application) {
  int32_t index;

  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    if (IVY_IS_VALID_WINDOW(&application->windows[index])) {
      return 0;
    }
  }

  return 1;
}

IVY_API void ivyPollApplicationEvents(IvyApplication *application) {
  int32_t index;

  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    IvyWindow *window = &application->windows[index];

    if (!IVY_IS_VALID_WINDOW(window)) {
      continue;
    }

    window->resized = 0;

    if (!glfwWindowShouldClose(window->opaque)) {
      continue;
    }

    ivyDestroyWindow(application, window);
  }

  glfwPollEvents();
}

#define IVY_MAX_DEBUG_VULKAN_INSTANCE_EXTENSIONS 64

IVY_API char const *const *ivyGetRequiredVulkanExtensions(
    IvyApplication *application, uint32_t *count) {
#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
  char const **defaultExtensions;
  static char const *extensions[IVY_MAX_DEBUG_VULKAN_INSTANCE_EXTENSIONS];

  IVY_UNUSED(application);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  defaultExtensions = glfwGetRequiredInstanceExtensions(count);
  if (!defaultExtensions) {
    *count = 0;
    return NULL;
  }

  if (IVY_MAX_DEBUG_VULKAN_INSTANCE_EXTENSIONS <= (*count + 3)) {
    *count = 0;
    return NULL;
  }

  IVY_MEMCPY(extensions, defaultExtensions,
      *count * sizeof(*defaultExtensions));

  extensions[(*count)++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

#if __APPLE__
  extensions[(*count)++] = "VK_KHR_portability_enumeration";
  extensions[(*count)++] = "VK_KHR_get_physical_device_properties2";
#endif

  return extensions;
#else  /* IVY_ENABLE_VULKAN_VALIDATION_LAYERS */
  IVY_ASSERT(does_an_application_already_exist);

  return glfwGetRequiredInstanceExtensions(num);
#endif /* IVY_ENABLE_VULKAN_VALIDATION_LAYERS */
}

IVY_API VkSurfaceKHR ivyCreateVulkanSurface(VkInstance instance,
    IvyApplication *application) {
  VkResult result;
  VkSurfaceKHR surface;
  IvyWindow *window;

  IVY_ASSERT(instance);
  IVY_ASSERT(application);

  window = application->lastAddedWindow;

  result = glfwCreateWindowSurface(instance, window->opaque, NULL, &surface);

  if (result) {
    return VK_NULL_HANDLE;
  }

  return surface;
}
