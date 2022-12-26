#ifndef IVY_GRAPHICS_PROGRAM_H
#define IVY_GRAPHICS_PROGRAM_H

#include <vulkan/vulkan.h>

#include "IvyGraphicsContext.h"

#define IVY_POLYGON_MODE_FILL 0x00000001
#define IVY_POLYGON_MODE_LINE 0x00000002
#define IVY_DEPTH_ENABLE 0x00000004
#define IVY_BLEND_ENABLE 0x00000008

typedef struct IvyGraphicsProgram {
  VkShaderModule vertexShader;
  VkShaderModule fragmentShader;
  VkPipeline     pipeline;
} IvyGraphicsProgram;

IvyCode ivyCreateGraphicsProgram(
    IvyGraphicsContext *context,
    int32_t             viewportWidth,
    int32_t             viewportHeight,
    int32_t             scissorWidth,
    int32_t             scissorHeight,
    char const         *vertexShaderPath,
    char const         *fragmentShaderPath,
    uint64_t            flags,
    IvyGraphicsProgram *program);

void ivyDestroyGraphicsProgram(
    IvyGraphicsContext *context,
    IvyGraphicsProgram *program);

#endif
