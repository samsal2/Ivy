#include "IvyGLFWApplication.h"

/* clang-format off */
#define GLFW_INCLUDE_NONE
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
/* clang-format on */

IVY_INTERNAL int doesAnApplicationAlreadyExist = 0;

#define IVY_IS_VALID_GLFW_WINDOW(w) (!!(w)->window)

IVY_INTERNAL void ivyGLFWWindowSizeCallback(GLFWwindow *opaque, int width,
    int height) {
  IvyGLFWWindow *window = glfwGetWindowUserPointer(opaque);
  window->windowWidth = width;
  window->windowHeight = height;
  window->resized = 1;
}

IVY_INTERNAL void ivyGLFWFramebufferSizeCallback(GLFWwindow *opaque, int width,
    int height) {
  IvyGLFWWindow *window = glfwGetWindowUserPointer(opaque);
  window->framebufferWidth = width;
  window->framebufferHeight = height;
}

IVY_INTERNAL void ivyInvalidateGLFWWindow(IvyGLFWWindow *window) {
  IVY_ASSERT(window);

  window->window = NULL;
  window->windowWidth = 0;
  window->windowHeight = 0;
  window->framebufferWidth = 0;
  window->framebufferHeight = 0;
}

IVY_API IvyCode ivyCreateGLFWApplication(IvyAnyMemoryAllocator allocator,
    IvyGLFWApplication **application) {
  int32_t index;
  IvyGLFWApplication *currentApplication;

  *application = NULL;

  IVY_ASSERT(allocator);

  if (doesAnApplicationAlreadyExist) {
    return IVY_ERROR_MORE_THAN_ONE_INSTANCE;
  }

  currentApplication =
      ivyAllocateMemory(allocator, sizeof(*currentApplication));
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
    ivyInvalidateGLFWWindow(&currentApplication->windows[index]);
  }

  doesAnApplicationAlreadyExist = 1;
  *application = currentApplication;

  return IVY_OK;
}

IVY_API void ivyDestroyGLFWApplication(IvyAnyMemoryAllocator allocator,
    IvyGLFWApplication *application) {
  if (!application) {
    return;
  }

  ivyPollGLFWApplicationEvents(application);
  glfwTerminate();
  ivyFreeMemory(allocator, application);
  doesAnApplicationAlreadyExist = 0;
}

IVY_API char const *ivyGetGLFWApplicationName(
    IvyGLFWApplication *application) {
  return application->name;
}

IVY_INTERNAL IvyGLFWWindow *ivyNextInvalidGLFWWindow(
    IvyGLFWApplication *application) {
  int32_t index;

  IVY_ASSERT(application);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    IvyGLFWWindow *window = &application->windows[index];

    if (!IVY_IS_VALID_GLFW_WINDOW(window)) {
      return window;
    }
  }

  return NULL;
}

IVY_API IvyGLFWWindow *ivyAddGLFWWindow(IvyGLFWApplication *application,
    int32_t width, int32_t height, char const *title) {
  IvyGLFWWindow *window;

  IVY_ASSERT(application);
  IVY_ASSERT(title);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  if (!application) {
    return NULL;
  }

  window = ivyNextInvalidGLFWWindow(application);
  if (!window) {
    return NULL;
  }

  window->window =
      glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
  if (!window->window) {
    return NULL;
  }

  glfwSetWindowUserPointer(window->window, window);
  glfwSetFramebufferSizeCallback(window->window,
      ivyGLFWFramebufferSizeCallback);
  glfwSetWindowSizeCallback(window->window, ivyGLFWWindowSizeCallback);

  window->resized = 0;
  glfwGetWindowSize(window->window, &window->windowWidth,
      &window->windowHeight);
  glfwGetFramebufferSize(window->window, &window->framebufferWidth,
      &window->framebufferHeight);

  return application->lastAddedWindow = window;
}

IVY_INTERNAL IvyBool ivyIsGLFWWindowInApplication(
    IvyGLFWApplication *application, IvyGLFWWindow *window) {
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

IVY_API IvyGLFWWindow *ivyGetFirstValidGLFWWindow(
    IvyGLFWApplication *application) {
  int32_t index;

  IVY_ASSERT(application);
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    IvyGLFWWindow *window = &application->windows[index];

    if (IVY_IS_VALID_GLFW_WINDOW(window)) {
      return window;
    }
  }

  return NULL;
}

IVY_API IvyCode ivyDestroyGLFWWindow(IvyGLFWApplication *application,
    IvyGLFWWindow *window) {
  IVY_ASSERT(doesAnApplicationAlreadyExist);

  if (!application || !window || !IVY_IS_VALID_GLFW_WINDOW(window)) {
    return IVY_ERROR_INVALID_VALUE;
  }

  if (!ivyIsGLFWWindowInApplication(application, window)) {
    return IVY_ERROR_INVALID_VALUE;
  }

  glfwDestroyWindow(window->window);
  window->window = NULL;

  // FIXME(samuel): use a FIFO or something
  application->lastAddedWindow = ivyGetFirstValidGLFWWindow(application);

  return IVY_OK;
}

IVY_API IvyBool ivyShouldGLFWApplicationClose(
    IvyGLFWApplication *application) {
  int32_t index;

  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    if (IVY_IS_VALID_GLFW_WINDOW(&application->windows[index])) {
      return 0;
    }
  }

  return 1;
}

IVY_API void ivyPollGLFWApplicationEvents(IvyGLFWApplication *application) {
  int32_t index;

  IVY_ASSERT(doesAnApplicationAlreadyExist);

  for (index = 0; index < IVY_ARRAY_LENGTH(application->windows); ++index) {
    IvyGLFWWindow *window = &application->windows[index];

    if (!IVY_IS_VALID_GLFW_WINDOW(window)) {
      continue;
    }

    window->resized = 0;

    if (!glfwWindowShouldClose(window->window)) {
      continue;
    }

    ivyDestroyGLFWWindow(application, window);
  }

  glfwPollEvents();
}

IVY_API void ivyGetGLFWApplicationFramebufferSize(
    IvyGLFWApplication *application, int32_t *width, int32_t *height) {
  *width = application->lastAddedWindow->framebufferWidth;
  *height = application->lastAddedWindow->framebufferHeight;
}

#define IVY_MAX_VULKAN_INSTANCE_EXTENSIONS 64

IVY_API char const *const *ivyGLFWGetRequiredVulkanExtensions(
    IvyGLFWApplication *application, uint32_t *count) {
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

IVY_API VkResult ivyGLFWCreateVulkanSurface(VkInstance instance,
    IvyGLFWApplication *application, VkSurfaceKHR *surface) {
  IVY_ASSERT(instance);
  IVY_ASSERT(application);

  return glfwCreateWindowSurface(instance,
      application->lastAddedWindow->window, NULL, surface);
}
