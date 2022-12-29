#include "IvyDraw.h"

static void ivyBindVertexGraphicsTemporaryBuffer(IvyRenderer *renderer,
    IvyGraphicsTemporaryBuffer *vertexBuffer) {
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, &vertexBuffer->buffer,
      &vertexBuffer->offset);
}

static void ivyBindIndexGraphicsTemporaryBuffer(IvyRenderer *renderer,
    IvyGraphicsTemporaryBuffer *indexBuffer) {
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  vkCmdBindIndexBuffer(frame->commandBuffer, indexBuffer->buffer,
      indexBuffer->offset, VK_INDEX_TYPE_UINT32);
}

static void ivyBindUniformGraphicsTemporaryBufferAndTexture(
    IvyRenderer *renderer, IvyGraphicsTemporaryBuffer *uniformBuffer,
    IvyGraphicsTexture *texture) {
  uint32_t offset = uniformBuffer->offset;
  VkDescriptorSet descriptorSets[2];
  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  descriptorSets[0] = uniformBuffer->descriptorSet;
  descriptorSets[1] = texture->descriptorSet;

  vkCmdBindDescriptorSets(frame->commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->mainPipelineLayout, 0,
      IVY_ARRAY_LENGTH(descriptorSets), descriptorSets, 1, &offset);
}

IvyCode ivyDrawRectangle(IvyRenderer *renderer, float x0, float y0, float x1,
    float y1, float r, float g, float b, IvyGraphicsTexture *texture) {
  IvyCode ivyCode;
  IvyGraphicsTemporaryBuffer vertexBuffer;
  IvyGraphicsTemporaryBuffer indexBuffer;
  IvyGraphicsTemporaryBuffer uniformBuffer;

  IvyGraphicsProgramUniform uniform;
  IvyGraphicsProgramVertex vertices[4];
  IvyGraphicsProgramIndex indices[] = {2, 3, 1, 1, 0, 2};

  IvyGraphicsFrame *frame = ivyGetCurrentGraphicsFrame(renderer);

  ivyCode = ivyRequestGraphicsTemporaryBufferFromRenderer(renderer,
      sizeof(vertices), &vertexBuffer);
  if (ivyCode)
    return ivyCode;

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
  vertices[1].uv[0] = 0.0F;
  vertices[1].uv[1] = 0.0F;

  vertices[2].position[0] = x0;
  vertices[2].position[1] = y1;
  vertices[2].position[2] = 0.0F;
  vertices[2].color[0] = r;
  vertices[2].color[1] = g;
  vertices[2].color[2] = b;
  vertices[2].uv[0] = 0.0F;
  vertices[2].uv[1] = 0.0F;

  vertices[3].position[0] = x1;
  vertices[3].position[1] = y1;
  vertices[3].position[2] = 0.0F;
  vertices[3].color[0] = r;
  vertices[3].color[1] = g;
  vertices[3].color[2] = b;
  vertices[3].uv[0] = 0.0F;
  vertices[3].uv[1] = 0.0F;

  IVY_MEMCPY(vertexBuffer.data, vertices, sizeof(vertices));

  ivyCode = ivyRequestGraphicsTemporaryBufferFromRenderer(renderer,
      sizeof(indices), &indexBuffer);
  if (ivyCode)
    return ivyCode;

  IVY_MEMCPY(indexBuffer.data, indices, sizeof(indices));

  ivyCode = ivyRequestGraphicsTemporaryBufferFromRenderer(renderer,
      sizeof(uniform), &uniformBuffer);
  if (ivyCode)
    return ivyCode;

  IVY_MEMSET(indexBuffer.data, 0, sizeof(uniform));

  ivyBindGraphicsProgramInRenderer(renderer, &renderer->basicGraphicsProgram);
  ivyBindVertexGraphicsTemporaryBuffer(renderer, &vertexBuffer);
  ivyBindIndexGraphicsTemporaryBuffer(renderer, &indexBuffer);
  ivyBindUniformGraphicsTemporaryBufferAndTexture(renderer, &uniformBuffer,
      texture);

  vkCmdDrawIndexed(frame->commandBuffer, IVY_ARRAY_LENGTH(indices), 1, 0, 0,
      0);

  return IVY_OK;
}
