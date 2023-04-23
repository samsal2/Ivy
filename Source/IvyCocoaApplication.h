#ifndef IVY_COCOA_APPLICATION_H
#define IVY_COCOA_APPLICATION_H

#include "IvyMemoryAllocator.h"

#include <vulkan/vulkan.h>

typedef struct IvyCocoaWindow {
    void *opaque;
} IvyCocoaWindow;

typedef struct IvyCocoaApplication {
  void *opaque;
} IvyCocoaApplication;

IVY_API IvyCode ivyCreateCocoaApplication(IvyAnyMemoryAllocator allocator,
    IvyCocoaApplication **application);

IVY_API void ivyDestroyCocoaApplication(IvyAnyMemoryAllocator allocator,
    IvyCocoaApplication *application);

IVY_API char const *ivyGetCocoaApplicationName(IvyCocoaApplication *application);

IVY_API IvyCocoaWindow *ivyAddCocoaWindow(IvyCocoaApplication *application,
    int32_t width, int32_t height, char const *title);

IVY_API IvyCode ivyDestroyCocoaWindow(IvyCocoaApplication *application,
    IvyCocoaWindow *window);

IVY_API IvyBool ivyShouldCocoaApplicationClose(IvyCocoaApplication *application);

IVY_API void ivyPollCocoaApplicationEvents(IvyCocoaApplication *application);

IVY_API void ivyGetCocoaApplicationFramebufferSize(
    IvyCocoaApplication *application, int32_t *width, int32_t *height);

IVY_API char const *const *ivyCocoaGetRequiredVulkanExtensions(
    IvyCocoaApplication *application, uint32_t *count);

IVY_API VkResult ivyCocoaCreateVulkanSurface(VkInstance instance,
    IvyCocoaApplication *application, VkSurfaceKHR *surface);

#endif
