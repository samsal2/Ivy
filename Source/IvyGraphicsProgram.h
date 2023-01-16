#ifndef IVY_GRAPHICS_PROGRAM_H
#define IVY_GRAPHICS_PROGRAM_H

#include <vulkan/vulkan.h>

#include "IvyMemoryAllocator.h"
#include "IvyVectorMath.h"

typedef enum IvyGraphicsProgramProperty {
  IVY_POLYGON_MODE_FILL = 0x00000001,
  IVY_POLYGON_MODE_LINE = 0x00000002,

  IVY_DEPTH_ENABLE = 0x00000004,
  IVY_BLEND_ENABLE = 0x00000008,

  IVY_CULL_FRONT = 0x00000010,
  IVY_CULL_BACK = 0x00000020,

  IVY_FRONT_FACE_COUNTERCLOCKWISE = 0x00000040,
  IVY_FRONT_FACE_CLOCKWISE = 0x00000080,

  IVY_VERTEX_3_ENABLE = 0x00000200,
  IVY_VERTEX_332_ENABLE = 0x00000400,
  IVY_VERTEX_3322444_ENABLE = 0x00000800,
  IVY_VERTEX_ENABLE_MASK = 0x00000F00
} IvyGraphicsProgramProperty;
typedef uint32_t IvyGraphicsProgramPropertyFlags;

typedef uint32_t IvyGraphicsProgramIndex;
typedef struct IvyGraphicsDevice IvyGraphicsDevice;

typedef struct IvyGraphicsVertex3 {
  IvyV3 position;
} IvyGraphicsVertex3;

typedef struct IvyGraphicsVertex332 {
  IvyV3 position;
  IvyV3 color;
  IvyV2 uv;
} IvyGraphicsVertex332;

typedef struct IvyGraphicsVertex3322444 {
  IvyV3 position;
  IvyV3 normal;
  IvyV2 uv0;
  IvyV2 uv1;
  IvyV4 joint0;
  IvyV4 weight0;
  IvyV4 color0;
} IvyGraphicsVertex3322444;

typedef struct IvyGraphicsProgramUniform {
  IvyM4 model;
  IvyM4 view;
  IvyM4 projection;
} IvyGraphicsProgramUniform;

typedef struct IvyGraphicsProgram {
  VkPipeline pipeline;
} IvyGraphicsProgram;

IVY_API IvyCode ivyCreateGraphicsProgram(IvyAnyMemoryAllocator allocator,
    IvyGraphicsDevice *device, VkSampleCountFlagBits samples,
    VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
    int32_t viewportWidth, int32_t viewportHeight,
    char const *vertexShaderPath, char const *fragmentShaderPath,
    IvyGraphicsProgramPropertyFlags flags, IvyGraphicsProgram *program);

IVY_API void ivyDestroyGraphicsProgram(IvyGraphicsDevice *device,
    IvyGraphicsProgram *program);

#endif
