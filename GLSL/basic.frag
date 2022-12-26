
#version 450

layout(set = 1, binding = 0) uniform texture2D texture0;
layout(set = 1, binding = 1) uniform sampler sampler0;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(inColor, 1.0) * texture(sampler2D(texture0, sampler0), inUV);
}
