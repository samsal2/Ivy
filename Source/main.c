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

  application = ivyCreateApplication(allocator);
  if (!application) {
    printf("failed to create application\n");
    goto error;
  }

  if (!ivyAddWindow(application, 600, 600, "hi")) {
    printf("failed to add window\n");
    goto error;
  }

  renderer = ivyCreateRenderer(allocator, application);
  if (!renderer) {
    printf("failed to create renderer\n");
    goto error;
  }

  texture = ivyCreateGraphicsTextureFromFile(allocator,
      renderer->graphicsContext, &renderer->defaultGraphicsMemoryAllocator,
      renderer->textureDescriptorSetLayout, "../Resources/Ivy.jpg");
  if (!texture) {
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
  ivyDestroyGraphicsTexture(allocator, renderer->graphicsContext,
      &renderer->defaultGraphicsMemoryAllocator, texture);
  ivyDestroyRenderer(allocator, renderer);
  ivyDestroyApplication(allocator, application);
  ivyDestroyGlobalMemoryAllocator();

  return 0;
}
