#ifndef IVY_APPLICATION_H
#define IVY_APPLICATION_H

#include "IvyDeclarations.h"

#include <vulkan/vulkan.h>

#define IVY_MAX_WINDOWS 4

struct ivy_graphics_context;

typedef struct IvyWindow {
  void   *opaque;
  int32_t windowWidth;
  int32_t windowHeight;
  int32_t framebufferWidth;
  int32_t framebufferHeight;
} IvyWindow;

typedef struct IvyApplication {
  void       *opaque;
  char const *name;
  IvyWindow  *lastAddedWindow;
  IvyWindow   windows[IVY_MAX_WINDOWS];
} IvyApplication;

IvyCode ivyCreateApplication(IvyApplication *application);
void    ivyDestroyApplication(IvyApplication *application);

IvyWindow *ivyAddWindow(
    IvyApplication *application,
    int32_t         width,
    int32_t         height,
    char const     *title);

IvyCode ivyDestroyWindow(IvyApplication *application, IvyWindow *window);

IvyBool ivyShouldApplicationClose(IvyApplication *application);

void ivyPollApplicationEvents(IvyApplication *application);

char const *const *
ivyGetRequiredVulkanExtensions(IvyApplication *application, uint32_t *num);

VkSurfaceKHR
ivyCreateVulkanSurface(VkInstance instance, IvyApplication *application);

#endif
