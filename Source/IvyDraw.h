#ifndef IVY_DRAW_H
#define IVY_DRAW_H

#include "IvyGraphicsTexture.h"
#include "IvyRenderer.h"

IVY_API IvyCode ivyDrawRectangle(IvyRenderer *renderer, float topLeftX,
    float topLeftY, float bottomRightX, float bottomRightY, float red,
    float green, float blue, IvyGraphicsTexture *texture);

#endif
