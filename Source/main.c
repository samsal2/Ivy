#include "IvyApplication.h"
#include "IvyGraphicsContext.h"

#include <stdio.h>

int main(void) {
  IvyApplication     app;
  IvyGraphicsContext context;

  if (0 > ivyCreateApplication(&app)) {
    printf("failed to create application\n");
    return 0;
  }

  if (0 > ivyAddWindow(&app, 600, 600, "hi")) {
    printf("failed to add window\n");
    return 0;
  }

  if (0 > ivyCreateGraphicsContext(&app, &context)) {
    printf("failed to create graphics_context\n");
    return 0;
  }

  while (!ivyShouldApplicationClose(&app))
    ivyPollApplicationEvents(&app);

  ivyDestroyGraphicsContext(&context);
  ivyDestroyApplication(&app);

  return 0;
}
