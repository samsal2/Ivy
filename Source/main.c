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
  IvyCode ivyCode;
  allocator = ivyGetGlobalMemoryAllocator();

  ivyCode = ivyCreateApplication(allocator, &application);
  if (ivyCode) {
    printf("failed to create application\n");
    goto error;
  }

  if (!ivyAddWindow(application, 600, 600, "hi")) {
    printf("failed to add window\n");
    goto error;
  }

  ivyCode = ivyCreateRenderer(allocator, application, &renderer);
  if (ivyCode) {
    printf("failed to create renderer, %i\n", ivyCode);
    goto error;
  }

  ivyCode = ivyCreateGraphicsTextureFromFile(allocator, renderer,
      "../Resources/Ivy.jpg", &texture);
  if (ivyCode) {
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
  ivyDestroyGraphicsTexture(allocator, renderer, texture);
  ivyDestroyRenderer(allocator, renderer);
  ivyDestroyApplication(allocator, application);
  ivyDestroyGlobalMemoryAllocator();

  return 0;
}
