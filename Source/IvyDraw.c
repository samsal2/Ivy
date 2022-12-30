#include "IvyDraw.h"

static IvyCode ivyBindGraphicsVertexData(IvyRenderer *renderer,
    uint64_t vertexCount, IvyGraphicsProgramVertex *vertices) {
  IvyCode ivyCode;
  IvyGraphicsTemporaryBuffer vertexBuffer;
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  ivyCode = ivyRequestGraphicsTemporaryBufferFromRenderer(renderer,
      vertexCount * sizeof(*vertices), &vertexBuffer);
  if (ivyCode) {
    return ivyCode;
  }

  IVY_MEMCPY(vertexBuffer.data, vertices, vertexBuffer.size);

  vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, &vertexBuffer.buffer,
      &vertexBuffer.offset);

  return IVY_OK;
}

static IvyCode ivyBindGraphicsIndexData(IvyRenderer *renderer,
    uint64_t indexCount, IvyGraphicsProgramIndex *indices) {
  IvyCode ivyCode;
  IvyGraphicsTemporaryBuffer indexBuffer;
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  ivyCode = ivyRequestGraphicsTemporaryBufferFromRenderer(renderer,
      indexCount * sizeof(*indices), &indexBuffer);
  if (ivyCode) {
    return ivyCode;
  }

  IVY_MEMCPY(indexBuffer.data, indices, indexBuffer.size);

  vkCmdBindIndexBuffer(frame->commandBuffer, indexBuffer.buffer,
      indexBuffer.offset, VK_INDEX_TYPE_UINT32);

  return IVY_OK;
}

static IvyCode IvyBindGraphicsUniformData(IvyRenderer *renderer,
    IvyGraphicsProgramUniform *uniform) {
  IvyCode ivyCode;
  uint32_t offset;
  IvyGraphicsTemporaryBuffer uniformBuffer;
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  ivyCode = ivyRequestGraphicsTemporaryBufferFromRenderer(renderer,
      sizeof(*uniform), &uniformBuffer);
  if (ivyCode) {
    return ivyCode;
  }

  IVY_MEMCPY(uniformBuffer.data, uniform, uniformBuffer.size);

  offset = uniformBuffer.offset;
  vkCmdBindDescriptorSets(frame->commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->mainPipelineLayout, 0, 1,
      &uniformBuffer.descriptorSet, 1, &offset);

  return IVY_OK;
}

static void ivyBindGraphicsTexture(IvyRenderer *renderer,
    IvyGraphicsTexture *texture) {
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);
  vkCmdBindDescriptorSets(frame->commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->mainPipelineLayout, 1, 1,
      &texture->descriptorSet, 0, NULL);
}

IvyCode ivyDrawRectangle(IvyRenderer *renderer, float x0, float y0, float x1,
    float y1, float r, float g, float b, IvyGraphicsTexture *texture) {
  IvyCode ivyCode;

  IvyGraphicsProgramUniform uniform;
  IvyGraphicsProgramVertex vertices[4];
  IvyGraphicsProgramIndex indices[] = {0, 2, 1, 2, 3, 1};

  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  vertices[0].position[0] = x0;
  vertices[0].position[1] = y0;
  vertices[0].position[2] = 0.0F;
  vertices[0].color[0] = r;
  vertices[0].color[1] = g;
  vertices[0].color[2] = b;
  vertices[0].uv[0] = 0.0F;
  vertices[0].uv[1] = 0.0F;

  vertices[1].position[0] = x1;
  vertices[1].position[1] = y0;
  vertices[1].position[2] = 0.0F;
  vertices[1].color[0] = r;
  vertices[1].color[1] = g;
  vertices[1].color[2] = b;
  vertices[1].uv[0] = 1.0F;
  vertices[1].uv[1] = 0.0F;

  vertices[2].position[0] = x0;
  vertices[2].position[1] = y1;
  vertices[2].position[2] = 0.0F;
  vertices[2].color[0] = r;
  vertices[2].color[1] = g;
  vertices[2].color[2] = b;
  vertices[2].uv[0] = 0.0F;
  vertices[2].uv[1] = 1.0F;

  vertices[3].position[0] = x1;
  vertices[3].position[1] = y1;
  vertices[3].position[2] = 0.0F;
  vertices[3].color[0] = r;
  vertices[3].color[1] = g;
  vertices[3].color[2] = b;
  vertices[3].uv[0] = 1.0F;
  vertices[3].uv[1] = 1.0F;

  ivyBindGraphicsProgramInRenderer(renderer, &renderer->basicGraphicsProgram);

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

#if 1
  vkCmdDrawIndexed(frame->commandBuffer, IVY_ARRAY_LENGTH(indices), 1, 0, 0,
      0);
#elif 1
  vkCmdDraw(frame->commandBuffer, IVY_ARRAY_LENGTH(vertices), 1, 0, 0);
#else
  vkCmdDraw(frame->commandBuffer, 3, 1, 0, 0);
#endif

  return IVY_OK;
}
