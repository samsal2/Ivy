#ifndef IVY_GRAPHICS_PROGRAM_H
#define IVY_GRAPHICS_PROGRAM_H

#include <vulkan/vulkan.h>

#include "IvyGraphicsContext.h"

#define IVY_POLYGON_MODE_FILL 0x00000001
#define IVY_POLYGON_MODE_LINE 0x00000002
#define IVY_DEPTH_ENABLE 0x00000004
#define IVY_BLEND_ENABLE 0x00000008
#define IVY_CULL_FRONT 0x00000010
#define IVY_CULL_BACK 0x00000020
#define IVY_FRONT_FACE_COUNTERCLOCKWISE 0x00000040
#define IVY_FRONT_FACE_CLOCKWISE 0x00000080

typedef struct IvyGraphicsProgramVertex {
  float position[3];
  float color[3];
  float uv[2];
} IvyGraphicsProgramVertex;

typedef struct IvyGraphicsProgram {
  VkShaderModule vertexShader;
  VkShaderModule fragmentShader;
  VkPipeline     pipeline;
} IvyGraphicsProgram;

IvyCode ivyCreateGraphicsProgram(
    IvyGraphicsContext *context,
    VkRenderPass        renderPass,
    VkPipelineLayout    pipelineLayout,
    int32_t             viewportWidth,
    int32_t             viewportHeight,
    char const         *vertexShaderPath,
    char const         *fragmentShaderPath,
    uint64_t            flags,
    IvyGraphicsProgram *program);

void ivyDestroyGraphicsProgram(
    IvyGraphicsContext *context,
    IvyGraphicsProgram *program);

#endif
