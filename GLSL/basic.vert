#version 450

layout(set = 0, binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
  mat4 model;
} ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

void main() {
  outColor = inColor;
  outUV = inUV;

#if 0
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
#elif 1
  gl_Position = vec4(inPosition, 1.0);
#else
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
#endif
}
