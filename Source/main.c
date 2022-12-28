#include "IvyApplication.h"
#include "IvyRenderer.h"

#include <stdio.h>

int main(void) {
  IvyApplication app;
  IvyRenderer    renderer;

  if (0 > ivyCreateApplication(&app)) {
    printf("failed to create application\n");
    return 0;
  }

  if (0 > ivyAddWindow(&app, 600, 600, "hi")) {
    printf("failed to add window\n");
    return 0;
  }

  if (0 > ivyCreateRenderer(&app, &renderer)) {
    printf("failed to create renderer\n");
    return 0;
  }

  while (!ivyShouldApplicationClose(&app)) {
    ivyBeginGraphicsFrame(&renderer);
    ivyEndGraphicsFrame(&renderer);
    ivyPollApplicationEvents(&app);
  }

  ivyDestroyRenderer(&renderer);
  ivyDestroyApplication(&app);

  return 0;
}
