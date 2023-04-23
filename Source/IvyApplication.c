#include "IvyApplication.h"

#if defined(IVY_USE_GLFW)
#include "IvyGLFWApplication.h"
#elif defined(IVY_USE_COCOA)
#include "IvyCocoaApplication.h"
#else
#error "Missing implementation"
#endif

IVY_API IvyCode ivyCreateApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication **application) {
#if defined(IVY_USE_GLFW)
  return ivyCreateGLFWApplication(allocator, application);
#elif defined(IVY_USE_COCOA)
  return ivyCreateCocoaApplication(allocator, application);
#endif
}

IVY_API void ivyDestroyApplication(IvyAnyMemoryAllocator allocator,
    IvyApplication *application) {
#if defined(IVY_USE_GLFW)
  ivyDestroyGLFWApplication(allocator, application);
#elif defined(IVY_USE_COCOA)
  ivyDestroyCocoaApplication(allocator, application);
#endif
}

IVY_API char const *ivyGetApplicationName(IvyApplication *application) {
#if defined(IVY_USE_GLFW)
  return ivyGetGLFWApplicationName(application);
#elif defined(IVY_USE_COCOA)
  return ivyGetCocoaApplicationName(application);
#endif
}

IVY_API IvyWindow *ivyAddWindow(IvyApplication *application, int32_t width,
    int32_t height, char const *title) {
#if defined(IVY_USE_GLFW)
  return ivyAddGLFWWindow(application, width, height, title);
#elif defined(IVY_USE_COCOA)
  return ivyAddCocoaWindow(application, width, height, title);
#endif
}

IVY_API IvyCode ivyDestroyWindow(IvyApplication *application,
    IvyWindow *window) {
#if defined(IVY_USE_GLFW)
  return ivyDestroyGLFWWindow(application, window);
#elif defined(IVY_USE_COCOA)
  return ivyDestroyCocoaWindow(application, window);
#endif
}

IVY_API IvyBool ivyShouldApplicationClose(IvyApplication *application) {
#if defined(IVY_USE_GLFW)
  return ivyShouldGLFWApplicationClose(application);
#elif defined(IVY_USE_COCOA)
  return ivyShouldCocoaApplicationClose(application);
#endif
}

IVY_API void ivyPollApplicationEvents(IvyApplication *application) {
#if defined(IVY_USE_GLFW)
  ivyPollGLFWApplicationEvents(application);
#elif defined(IVY_USE_COCOA)
  ivyPollCocoaApplicationEvents(application);
#endif
}

IVY_API void ivyGetApplicationFramebufferSize(IvyApplication *application,
    int32_t *width, int32_t *height) {
#if defined(IVY_USE_GLFW)
  ivyGetGLFWApplicationFramebufferSize(application, width, height);
#elif defined(IVY_USE_COCOA)
  ivyGetCocoaApplicationFramebufferSize(application, width, height);
#endif
}

IVY_API char const *const *ivyGetRequiredVulkanExtensions(
    IvyApplication *application, uint32_t *count) {
#if defined(IVY_USE_GLFW)
  return ivyGLFWGetRequiredVulkanExtensions(application, count);
#elif defined(IVY_USE_COCOA)
  return ivyCocoaGetRequiredVulkanExtensions(application, count);
#endif
}

IVY_API VkResult ivyCreateVulkanSurface(VkInstance instance,
    IvyApplication *application, VkSurfaceKHR *surface) {
#if defined(IVY_USE_GLFW)
  return ivyGLFWCreateVulkanSurface(instance, application, surface);
#elif defined(IVY_USE_COCOA)
  return ivyCocoaCreateVulkanSurface(instance, application, surface);
#endif
}
