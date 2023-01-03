#ifndef IVY_IMGUI_H
#define IVY_IMGUI_H

#include "IvyRenderer.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

void ivyStartupImGui(IvyRenderer *renderer);
void ivyShutdownImGui(IvyRenderer *renderer);

#endif
