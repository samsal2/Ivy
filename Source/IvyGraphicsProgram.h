#ifndef IVY_GRAPHICS_PROGRAM_H
#define IVY_GRAPHICS_PROGRAM_H

#include <vulkan/vulkan.h>

#include "IvyMemoryAllocator.h"
#include "IvyVectorMath.h"

typedef enum IvyGraphicsProgramProperty {
  IVY_GRAPHICS_PROGRAM_PROPERTY_POLYGON_MODE_FILL = 0x00000001,
  IVY_GRAPHICS_PROGRAM_PROPERTY_POLYGON_MODE_LINE = 0x00000002,
  IVY_GRAPHICS_PROGRAM_PROPERTY_DEPTH_ENABLE = 0x00000004,
  IVY_GRAPHICS_PROGRAM_PROPERTY_BLEND_ENABLE = 0x00000008,
  IVY_GRAPHICS_PROGRAM_PROPERTY_CULL_FRONT = 0x00000010,
  IVY_GRAPHICS_PROGRAM_PROPERTY_CULL_BACK = 0x00000020,
  IVY_GRAPHICS_PROGRAM_PROPERTY_FRONT_FACE_COUNTERCLOCKWISE = 0x00000040,
  IVY_GRAPHICS_PROGRAM_PROPERTY_FRONT_FACE_CLOCKWISE = 0x00000080
} IvyGraphicsProgramProperty;
typedef uint32_t IvyGraphicsProgramPropertyFlags;

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
    IvyGraphicsProgramPropertyFlags flags, IvyGraphicsProgram *program);

IVY_API void ivyDestroyGraphicsProgram(IvyGraphicsDevice *device,
    IvyGraphicsProgram *program);

#endif
