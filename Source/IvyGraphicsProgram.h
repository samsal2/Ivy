#ifndef IVY_GRAPHICS_PROGRAM_H
#define IVY_GRAPHICS_PROGRAM_H

#include <vulkan/vulkan.h>

#include "IvyMemoryAllocator.h"
#include "IvyVectorMath.h"

#define IVY_POLYGON_MODE_FILL 0x00000001
#define IVY_POLYGON_MODE_LINE 0x00000002
#define IVY_DEPTH_ENABLE 0x00000004
#define IVY_BLEND_ENABLE 0x00000008
#define IVY_CULL_FRONT 0x00000010
#define IVY_CULL_BACK 0x00000020
#define IVY_FRONT_FACE_COUNTERCLOCKWISE 0x00000040
#define IVY_FRONT_FACE_CLOCKWISE 0x00000080

typedef uint32_t IvyGraphicsProgramIndex;
typedef struct IvyGraphicsDevice IvyGraphicsDevice;

typedef struct IvyGraphicsProgramVertex {
  IvyV3 position;
  IvyV3 color;
  IvyV2 uv;
} IvyGraphicsProgramVertex;

typedef struct IvyGraphicsProgramUniform {
  IvyM4 model;
  IvyM4 view;
  IvyM4 projection;
} IvyGraphicsProgramUniform;

typedef struct IvyGraphicsProgram {
  VkShaderModule vertexShader;
  VkShaderModule fragmentShader;
  VkPipeline pipeline;
} IvyGraphicsProgram;

IVY_API IvyCode ivyCreateGraphicsProgram(IvyAnyMemoryAllocator allocator,
    IvyGraphicsDevice *device, VkSampleCountFlagBits samples,
    VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
    int32_t viewportWidth, int32_t viewportHeight,
    char const *vertexShaderPath, char const *fragmentShaderPath,
    uint64_t flags, IvyGraphicsProgram *program);

IVY_API void ivyDestroyGraphicsProgram(IvyGraphicsDevice *device,
    IvyGraphicsProgram *program);

#endif
