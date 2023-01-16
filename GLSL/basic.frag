#version 450

// layout(set = 0, binding = 1) uniform samplerCube samplerIrradiance;
// layout(set = 0, binding = 2) uniform samplerCube prefilteredMap;
// layout(set = 0, binding = 3) uniform sampler2D samplerBRDFLUT 

layout(set = 1, binding = 0) uniform sampler2D texture0;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(inColor, 1.0) * texture(texture0, inUV);
}
