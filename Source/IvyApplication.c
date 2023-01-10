#include "IvyApplication.h"

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

IVY_API IvyCode ivyCreateApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication **application) {
  int32_t index;
  IvyApplication *currentApplication;

  *application = NULL;

  IVY_ASSERT(allocator);

  if (doesAnApplicationAlreadyExist) {
    return IVY_ERROR_MORE_THAN_ONE_INSTANCE;
  }

  currentApplication = ivyAllocateMemory(allocator, sizeof(**application));
  if (!currentApplication) {
    return IVY_ERROR_NO_MEMORY;
  }

  currentApplication->opaque = NULL;
  currentApplication->lastAddedWindow = NULL;

  if (!glfwInit()) {
    ivyFreeMemory(allocator, application);
    return IVY_ERROR_UNKNOWN;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  for (index = 0; index < IVY_ARRAY_LENGTH(currentApplication->windows);
       ++index) {
    ivyInvalidateWindow(&currentApplication->windows[index]);
  }

  doesAnApplicationAlreadyExist = 1;
  *application = currentApplication;

  return IVY_OK;
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
    return IVY_ERROR_INVALID_VALUE;
  }

  if (!ivyIsWindowInApplication(application, window)) {
    return IVY_ERROR_INVALID_VALUE;
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

#define IVY_MAX_VULKAN_INSTANCE_EXTENSIONS 64

IVY_API char const *const *ivyGetRequiredVulkanExtensions(
    IvyApplication *application, uint32_t *count) {
  char const **defaultExtensions;
  IVY_LOCAL_PERSIST char const *extensions[IVY_MAX_VULKAN_INSTANCE_EXTENSIONS];

  IVY_UNUSED(application);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  defaultExtensions = glfwGetRequiredInstanceExtensions(count);
  if (!defaultExtensions) {
    *count = 0;
    return NULL;
  }

  if (IVY_MAX_VULKAN_INSTANCE_EXTENSIONS <= (*count + 3)) {
    *count = 0;
    return NULL;
  }

  IVY_MEMCPY(extensions, defaultExtensions,
      *count * sizeof(*defaultExtensions));

#ifdef IVY_ENABLE_VULKAN_VALIDATION_LAYERS
  extensions[(*count)++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

#if __APPLE__
  extensions[(*count)++] = "VK_KHR_portability_enumeration";
  extensions[(*count)++] = "VK_KHR_get_physical_device_properties2";
#endif

  return extensions;
}

IVY_API VkResult ivyCreateVulkanSurface(VkInstance instance,
    IvyApplication *application, VkSurfaceKHR *surface) {
  IVY_ASSERT(instance);
  IVY_ASSERT(application);

  return glfwCreateWindowSurface(instance,
      application->lastAddedWindow->opaque, NULL, surface);
}
