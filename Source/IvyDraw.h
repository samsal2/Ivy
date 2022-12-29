#ifndef IVY_DRAW_H
#define IVY_DRAW_H

#include "IvyGraphicsTexture.h"
#include "IvyRenderer.h"

IvyCode ivyDrawRectangle(
    IvyRenderer        *renderer,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               r,
    float               g,
    float               b,
    IvyGraphicsTexture *texture);

#endif
