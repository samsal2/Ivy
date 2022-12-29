#include "IvyApplication.h"
#include "IvyDraw.h"
#include "IvyGraphicsTexture.h"
#include "IvyRenderer.h"

#include <stdio.h>

IvyApplication app;
IvyRenderer renderer;
IvyGraphicsTexture texture;

int main(void) {

  if (0 > ivyCreateApplication(&app)) {
    printf("failed to create application\n");
    goto error;
  }

  if (!ivyAddWindow(&app, 600, 600, "hi")) {
    printf("failed to add window\n");
    goto error;
  }

  if (0 > ivyCreateRenderer(&app, &renderer)) {
    printf("failed to create renderer\n");
    goto error;
  }

  if (0 > ivyCreateGraphicsTextureFromFile(&renderer.graphicsContext,
              &renderer.defaultGraphicsMemoryAllocator,
              renderer.textureDescriptorSetLayout, "../Resources/Ivy.jpg",
              &texture)) {
    printf("failed to create texture\n");
    goto error;
  }

  while (!ivyShouldApplicationClose(&app)) {
    ivyBeginGraphicsFrame(&renderer);
    ivyDrawRectangle(&renderer, -1, -1, 1, 1, 1, 1, 1, &texture);
    ivyEndGraphicsFrame(&renderer);
    ivyPollApplicationEvents(&app);
  }

error:
  ivyDestroyGraphicsTexture(&renderer.graphicsContext,
      &renderer.defaultGraphicsMemoryAllocator, &texture);
  ivyDestroyRenderer(&renderer);
  ivyDestroyApplication(&app);

  return 0;
}
