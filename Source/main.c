#include "IvyApplication.h"
#include "IvyDraw.h"
#include "IvyGraphicsTexture.h"
#include "IvyMemoryAllocator.h"
#include "IvyRenderer.h"

#include <stdio.h>

IvyAnyMemoryAllocator allocator;
IvyApplication *application;
IvyRenderer *renderer;
IvyGraphicsTexture *texture;

int main(void) {
  allocator = ivyGetGlobalMemoryAllocator();

  application = ivyAllocateMemory(allocator, sizeof(*application));
  renderer = ivyAllocateMemory(allocator, sizeof(*renderer));
  texture = ivyAllocateMemory(allocator, sizeof(*texture));

  if (!application || !renderer || !texture) {
    printf("failed to allocate resources\n");
    goto error;
  }

  if (0 > ivyCreateApplication(application)) {
    printf("failed to create application\n");
    goto error;
  }

  if (!ivyAddWindow(application, 600, 600, "hi")) {
    printf("failed to add window\n");
    goto error;
  }

  if (0 > ivyCreateRenderer(application, renderer)) {
    printf("failed to create renderer\n");
    goto error;
  }

  if (0 > ivyCreateGraphicsTextureFromFile(&renderer->graphicsContext,
              &renderer->defaultGraphicsMemoryAllocator,
              renderer->textureDescriptorSetLayout, "../Resources/Ivy.jpg",
              texture)) {
    printf("failed to create texture\n");
    goto error;
  }

  while (!ivyShouldApplicationClose(application)) {
    ivyBeginGraphicsFrame(renderer);
    ivyDrawRectangle(renderer, -1, -1, 1, 1, 1, 1, 1, texture);
    ivyEndGraphicsFrame(renderer);
    ivyPollApplicationEvents(application);
  }

error:
  ivyDestroyGraphicsTexture(&renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator, texture);
  ivyDestroyRenderer(renderer);
  ivyDestroyApplication(application);

  ivyFreeMemory(allocator, texture);
  ivyFreeMemory(allocator, renderer);
  ivyFreeMemory(allocator, application);

  ivyDestroyGlobalMemoryAllocator();

  return 0;
}
