#ifndef IVY_APPLICATION_H
#define IVY_APPLICATION_H

#include "IvyDeclarations.h"
#include "IvyMemoryAllocator.h"

#include <vulkan/vulkan.h>

#if defined(IVY_USE_GLFW)
typedef struct IvyGLFWApplication IvyApplication;
typedef struct IvyGLFWWindow IvyWindow;
#elif defined (IVY_USE_COCOA)
typedef struct IvyCocoaApplication IvyApplication;
typedef struct IvyCocoaWindow IvyWindow;
#endif

#define IVY_MAX_WINDOWS 4

IVY_API IvyCode ivyCreateApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication **application);

IVY_API void ivyDestroyApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication *application);

IVY_API char const *ivyGetApplicationName(IvyApplication *application);

IVY_API IvyWindow *ivyAddWindow(IvyApplication *application, int32_t width,
    int32_t height, char const *title);

IVY_API IvyCode ivyDestroyWindow(IvyApplication *application,
    IvyWindow *window);

IVY_API IvyBool ivyShouldApplicationClose(IvyApplication *application);

IVY_API void ivyPollApplicationEvents(IvyApplication *application);

IVY_API void ivyGetApplicationFramebufferSize(IvyApplication *application,
    int32_t *width, int32_t *height);

IVY_API char const *const *ivyGetRequiredVulkanExtensions(
    IvyApplication *application, uint32_t *count);

IVY_API VkResult ivyCreateVulkanSurface(VkInstance instance,
    IvyApplication *application, VkSurfaceKHR *surface);

#endif
