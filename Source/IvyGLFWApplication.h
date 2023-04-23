#ifndef IVY_GLFW_APPLICATION_H
#define IVY_GLFW_APPLICATION_H

#include "IvyDeclarations.h"
#include "IvyMemoryAllocator.h"

/* clang-format off */
#define GLFW_INCLUDE_NONE
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
/* clang-format on */

#define IVY_MAX_WINDOWS 4

typedef struct IvyGLFWWindow {
  GLFWwindow *window;
  IvyBool resized;
  int32_t windowWidth;
  int32_t windowHeight;
  int32_t framebufferWidth;
  int32_t framebufferHeight;
} IvyGLFWWindow;

typedef struct IvyGLFWApplication {
  void *opaque;
  char const *name;
  IvyGLFWWindow *lastAddedWindow;
  IvyGLFWWindow windows[IVY_MAX_WINDOWS];
} IvyGLFWApplication;

IVY_API IvyCode ivyCreateGLFWApplication(IvyAnyMemoryAllocator allocator,
    IvyGLFWApplication **application);

IVY_API void ivyDestroyGLFWApplication(IvyAnyMemoryAllocator allocator,
    IvyGLFWApplication *application);

IVY_API char const *ivyGetGLFWApplicationName(IvyGLFWApplication *application);

IVY_API IvyGLFWWindow *ivyAddGLFWWindow(IvyGLFWApplication *application,
    int32_t width, int32_t height, char const *title);

IVY_API IvyCode ivyDestroyGLFWWindow(IvyGLFWApplication *application,
    IvyGLFWWindow *window);

IVY_API IvyBool ivyShouldGLFWApplicationClose(IvyGLFWApplication *application);

IVY_API void ivyPollGLFWApplicationEvents(IvyGLFWApplication *application);

IVY_API void ivyGetGLFWApplicationFramebufferSize(
    IvyGLFWApplication *application, int32_t *width, int32_t *height);

IVY_API char const *const *ivyGLFWGetRequiredVulkanExtensions(
    IvyGLFWApplication *application, uint32_t *count);

IVY_API VkResult ivyGLFWCreateVulkanSurface(VkInstance instance,
    IvyGLFWApplication *application, VkSurfaceKHR *surface);

#endif
