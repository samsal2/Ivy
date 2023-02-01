#include "IvyApplication.h"
#include "IvyDraw.h"
#include "IvyGraphicsTexture.h"
#include "IvyMemoryAllocator.h"
#include "IvyRenderer.h"

#include <stdio.h>

IvyAnyMemoryAllocator allocator;
IvyApplication *application = NULL;
IvyWindow *window = NULL;
IvyRenderer *renderer = NULL;
IvyGraphicsTexture *texture = NULL;

int main(void) {
  int iterationDirection = 1;
  int iteration = 0;
  float r = 1.0F;
  IvyCode ivyCode;
  allocator = ivyGetGlobalMemoryAllocator();

  ivyCode = ivyCreateApplication(allocator, &application);
  if (ivyCode) {
    printf("failed to create application\n");
    goto error;
  }

  window = ivyAddWindow(application, 600, 600, "hi");
  if (!window) {
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

    iteration += iterationDirection;
    if (iteration == 60)
      iterationDirection *= -1;
    else if (iteration == 0)
      iterationDirection *= -1;

    r = ((float)iteration) / 60.0F;

    ivyDrawRectangle(renderer, -1, -1, 0, 0, r, 1, 1, texture);
    ivyDrawRectangle(renderer, 0, 0, 1, 1, r, 1, r, texture);

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
