#include "IvyDraw.h"

#include "IvyLog.h"

IVY_INTERNAL IvyCode ivyBindGraphicsVertexData(IvyRenderer *renderer,
    uint64_t vertexCount, IvyGraphicsProgramVertex *vertices) {
  IvyCode ivyCode;
  IvyGraphicsTemporaryBuffer vertexBuffer;
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  ivyCode = ivyRequestGraphicsTemporaryBuffer(renderer,
      vertexCount * sizeof(*vertices), &vertexBuffer);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    return ivyCode;
  }

  IVY_MEMCPY(vertexBuffer.data, vertices, vertexBuffer.size);

  vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, &vertexBuffer.buffer,
      &vertexBuffer.offsetInU64);

  return IVY_OK;
}

IVY_INTERNAL IvyCode ivyBindGraphicsIndexData(IvyRenderer *renderer,
    uint64_t indexCount, IvyGraphicsProgramIndex *indices) {
  IvyCode ivyCode;
  IvyGraphicsTemporaryBuffer indexBuffer;
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  ivyCode = ivyRequestGraphicsTemporaryBuffer(renderer,
      indexCount * sizeof(*indices), &indexBuffer);
  if (ivyCode) {
    return ivyCode;
  }

  IVY_MEMCPY(indexBuffer.data, indices, indexBuffer.size);

  vkCmdBindIndexBuffer(frame->commandBuffer, indexBuffer.buffer,
      indexBuffer.offsetInU32, VK_INDEX_TYPE_UINT32);

  return IVY_OK;
}

IVY_INTERNAL IvyCode IvyBindGraphicsUniformData(IvyRenderer *renderer,
    IvyGraphicsProgramUniform *uniform) {
  IvyCode ivyCode;
  IvyGraphicsTemporaryBuffer uniformBuffer;
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  ivyCode = ivyRequestGraphicsTemporaryBuffer(renderer, sizeof(*uniform),
      &uniformBuffer);
  if (ivyCode) {
    return ivyCode;
  }

  IVY_MEMCPY(uniformBuffer.data, uniform, uniformBuffer.size);

  vkCmdBindDescriptorSets(frame->commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->mainPipelineLayout, 0, 1,
      &uniformBuffer.descriptorSet, 1, &uniformBuffer.offsetInU32);

  return IVY_OK;
}

IVY_INTERNAL void ivyBindGraphicsTexture(IvyRenderer *renderer,
    IvyGraphicsTexture *texture) {
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);
  vkCmdBindDescriptorSets(frame->commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->mainPipelineLayout, 1, 1,
      &texture->descriptorSet, 0, NULL);
}

IVY_API IvyCode ivyDrawRectangle(IvyRenderer *renderer, float topLeftX,
    float topLeftY, float bottomRightX, float bottomRightY, float red,
    float green, float blue, IvyGraphicsTexture *texture) {
  IvyCode ivyCode;

  IvyGraphicsProgramUniform uniform;
  IvyGraphicsProgramVertex vertices[4];
  IvyGraphicsProgramIndex indices[] = {0, 2, 1, 2, 3, 1};

  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  vertices[0].position[0] = topLeftX;
  vertices[0].position[1] = topLeftY;
  vertices[0].position[2] = 0.0F;
  vertices[0].color[0] = red;
  vertices[0].color[1] = green;
  vertices[0].color[2] = blue;
  vertices[0].uv[0] = 0.0F;
  vertices[0].uv[1] = 0.0F;

#if 0
  IVY_DEBUG_LOG(" red: %.2f\n"#include "IvyLog.h", red);
  IVY_DEBUG_LOG(" green: %.2f\n", green);
  IVY_DEBUG_LOG(" blue: %.2f\n", blue);
#endif

  vertices[1].position[0] = bottomRightX;
  vertices[1].position[1] = topLeftY;
  vertices[1].position[2] = 0.0F;
  vertices[1].color[0] = red;
  vertices[1].color[1] = green;
  vertices[1].color[2] = blue;
  vertices[1].uv[0] = 1.0F;
  vertices[1].uv[1] = 0.0F;

  vertices[2].position[0] = topLeftX;
  vertices[2].position[1] = bottomRightY;
  vertices[2].position[2] = 0.0F;
  vertices[2].color[0] = red;
  vertices[2].color[1] = green;
  vertices[2].color[2] = blue;
  vertices[2].uv[0] = 0.0F;
  vertices[2].uv[1] = 1.0F;

  vertices[3].position[0] = bottomRightX;
  vertices[3].position[1] = bottomRightY;
  vertices[3].position[2] = 0.0F;
  vertices[3].color[0] = red;
  vertices[3].color[1] = green;
  vertices[3].color[2] = blue;
  vertices[3].uv[0] = 1.0F;
  vertices[3].uv[1] = 1.0F;

  ivyBindGraphicsProgram(renderer, &renderer->basicGraphicsProgram);

  ivyCode = ivyBindGraphicsVertexData(renderer, IVY_ARRAY_LENGTH(vertices),
      vertices);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    return ivyCode;
  }

  ivyCode =
      ivyBindGraphicsIndexData(renderer, IVY_ARRAY_LENGTH(indices), indices);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    return ivyCode;
  }

  IVY_MEMSET(&uniform, 0, sizeof(uniform));
  ivyCode = IvyBindGraphicsUniformData(renderer, &uniform);
  IVY_ASSERT(!ivyCode);
  if (ivyCode) {
    return ivyCode;
  }

  ivyBindGraphicsTexture(renderer, texture);

  vkCmdDrawIndexed(frame->commandBuffer, IVY_ARRAY_LENGTH(indices), 1, 0, 0,
      0);

  return IVY_OK;
}
