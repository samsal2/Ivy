#ifndef IVY_APPLICATION_H
#define IVY_APPLICATION_H

#include "IvyDeclarations.h"
#include "IvyMemoryAllocator.h"

#include <vulkan/vulkan.h>

#define IVY_MAX_WINDOWS 4

typedef struct IvyWindow {
  void *opaque;
  IvyBool resized;
  int32_t windowWidth;
  int32_t windowHeight;
  int32_t framebufferWidth;
  int32_t framebufferHeight;
} IvyWindow;

typedef struct IvyApplication {
  void *opaque;
  char const *name;
  IvyWindow *lastAddedWindow;
  IvyWindow windows[IVY_MAX_WINDOWS];
} IvyApplication;

IVY_API IvyCode ivyCreateApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication **application);

IVY_API void ivyDestroyApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication *application);

IVY_API IvyWindow *ivyAddWindow(IvyApplication *application, int32_t width,
    int32_t height, char const *title);

IVY_API IvyCode ivyDestroyWindow(IvyApplication *application,
    IvyWindow *window);

IVY_API IvyBool ivyShouldApplicationClose(IvyApplication *application);

IVY_API void ivyPollApplicationEvents(IvyApplication *application);

IVY_API char const *const *ivyGetRequiredVulkanExtensions(
    IvyApplication *application, uint32_t *count);

IVY_API VkResult ivyCreateVulkanSurface(VkInstance instance,
    IvyApplication *application, VkSurfaceKHR *surface);

#endif
