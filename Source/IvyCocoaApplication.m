#include "IvyCocoaApplication.h"

#include <vulkan/vulkan.h>
#include <Cocoa/Cocoa.h>


@interface IvyApplicationDelegate : NSObject<NSApplicationDelegate>
{
}
@end

@implementation IvyApplicationDelegate 
{
}
@end


IVY_INTERNAL int doesAnApplicationAlreadyExist = 0;

IVY_INTERNAL char const * const requiredCocoaVulkanExtensions[] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	// VK_MVK_MACOS_SURFACE_EXTENSION_NAME, 
	"VK_MVK_macos_surface",
	VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
};

IVY_API IvyCode ivyCreateCocoaApplication(IvyAnyMemoryAllocator allocator,
    IvyCocoaApplication **application) {

	IVY_UNUSED(allocator);

  *application = NULL;

	if (doesAnApplicationAlreadyExist) {
    return IVY_ERROR_MORE_THAN_ONE_INSTANCE;
	}

	NSApp = [NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];




	return IVY_OK;
}

IVY_API void ivyDestroyCocoaApplication(IvyAnyMemoryAllocator allocator,
    IvyCocoaApplication *application) {
	ivyFreeMemory(allocator, application);
}

IVY_API char const *ivyGetCocoaApplicationName(IvyCocoaApplication *application)
{
	IVY_UNUSED(application);
	IVY_TODO();
	return "";
}

IVY_API IvyCocoaWindow *ivyAddCocoaWindow(IvyCocoaApplication *application,
    int32_t width, int32_t height, char const *title)
{
	IVY_UNUSED(application);
	IVY_UNUSED(width);
	IVY_UNUSED(height);
	IVY_UNUSED(title);
	IVY_TODO();
}


IVY_API IvyCode ivyDestroyCocoaWindow(IvyCocoaApplication *application,
    IvyCocoaWindow *window)
{
	IVY_UNUSED(application);
	IVY_UNUSED(window);
	IVY_TODO();
}

IVY_API IvyBool ivyShouldCocoaApplicationClose(IvyCocoaApplication *application)
{
	IVY_UNUSED(application);
	IVY_TODO();
	return 1;
}

IVY_API void ivyPollCocoaApplicationEvents(IvyCocoaApplication *application)
{
	IVY_UNUSED(application);
	IVY_TODO();
}

IVY_API void ivyGetCocoaApplicationFramebufferSize(
    IvyCocoaApplication *application, int32_t *width, int32_t *height)
{
	IVY_UNUSED(application);
	IVY_UNUSED(width);
	IVY_UNUSED(height);
	IVY_TODO();
}

IVY_API char const *const *ivyCocoaGetRequiredVulkanExtensions(
    IvyCocoaApplication *application, uint32_t *count) {
	IVY_UNUSED(application);
	*count = IVY_ARRAY_LENGTH(requiredCocoaVulkanExtensions);
	return requiredCocoaVulkanExtensions;
}

IVY_API VkResult ivyCocoaCreateVulkanSurface(VkInstance instance,
    IvyCocoaApplication *application, VkSurfaceKHR *surface)
{
	IVY_UNUSED(instance);
	IVY_UNUSED(application);
	IVY_UNUSED(surface);
	IVY_TODO();
	return VK_ERROR_UNKNOWN;
}
