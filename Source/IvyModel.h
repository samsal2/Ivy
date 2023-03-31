#ifndef IVY_MODEL_H
#define IVY_MODEL_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyMemoryAllocator.h"
#include "IvyVectorMath.h"

typedef struct IvyGraphicsTexture IvyGraphicsTexture;
typedef struct IvyGraphicsVertexBuffer IvyGraphicsVertexBuffer;
typedef struct IvyGraphicsIndexBuffer IvyGraphicsIndexBuffer;

typedef struct IvyBoundingBox {
  IvyV3 min;
  IvyV3 max;
  IvyBool isValid;
} IvyBoundingBox;

typedef enum IvyModelMaterialAlphaMode {
  IVY_MODEL_MATERIAL_ALPHA_MODE_OPAQUE = 0,
  IVY_MODEL_MATERIAL_ALPHA_MODE_MASK = 1,
  IVY_MODEL_MATERIAL_ALPHA_MODE_BLEND = 2,
} IvyModelMaterialAlphaMode;

typedef struct IvyModelMaterial {
  IvyModelMaterialAlphaMode alphaMode;
  float alphaCutoff;
  float metallicFactor;
  float roughnessFactor;
  IvyV4 baseColorFactor;
  IvyV4 emissiveFactor;
  int32_t baseColorTextureIndex;
  int32_t metallicRoughnessTextureIndex;
  int32_t isDoubleSided;
  int32_t baseColorTexcoord;
  int32_t metallicRoughnessTexcoord;
  int32_t normalTextureIndex;
  int32_t normalTexcoord;
  int32_t occlusionTextureIndex;
  int32_t occlusionTexcoord;
  int32_t emissiveTextureIndex;
  int32_t emissiveTexcoord;
  int32_t specularGlossinessTextureIndex;
  int32_t specularGlossinessTexcoord;
  int32_t diffuseTextureIndex;
  IvyV4 diffuseFactor;
  IvyV4 specularFactor;
  int32_t enableMetallicRoughness;
  int32_t enableSpecularGlossiness;
} IvyModelMaterial;

typedef struct IvyModelPrimitive {
  uint32_t first;
  uint32_t indexCount;
  uint32_t vertexCount;
  int32_t materialIndex;
} IvyModelPrimitive;

typedef struct IvyModelMappedSSBO {
  IvyM4 matrix;
  IvyM4 jointMatrices[128];
  int32_t jointMatricesCount;
} IvyModelMappedSSBO;

typedef struct IvyModelSSBO {
  IvyModelMappedSSBO *mappedSSBO;
  VkDescriptorSet descriptorSet;
  VkBuffer buffer;
  IvyGraphicsMemory memory;
} IvyModelSSBO;

typedef struct IvyModelMesh {
  int32_t primitiveCount;
  IvyModelPrimitive *primitives;
  int32_t ssboCount;
  IvyModelSSBO *ssbos;
} IvyModelMesh;

typedef struct IvyModelSkin {
  char name[256];
  int32_t rootNodeIndex;
  int32_t inverseBindMatrixCount;
  IvyM4 *inverseBindMatrices;
  int32_t jointsNodeIndexCount;
  int32_t *jointsNodeIndices;
} IvyModelSkin;

typedef struct IvyModelNode {
  char name[256];
  int32_t parentNodeIndex;
  int32_t childrenNodexIndexCount;
  int32_t *childrenNodeIndices;
  int32_t meshIndex;
  int32_t skinIndex;
  IvyM4 matrix;
  IvyV3 translation;
  IvyV3 scale;
  IvyV4 rotation;
  IvyBoundingBox bvh;
  IvyBoundingBox aabb;
} IvyModelNode;

typedef enum IvyModelAnimationPath {
  IVY_MODEL_ANIMATION_PATH_TRANSLATION = 1,
  IVY_MODEL_ANIMATION_PATH_ROTATION = 2,
  IVY_MODEL_ANIMATION_PATH_SCALE = 3,
  IVY_MODEL_ANIMATION_PATH_WEIGHTS = 4
} IvyModelAnimationPath;

typedef struct IvyModelAnimationChannel {
  IvyModelAnimationPath path;
  int32_t nodeIndex;
  int32_t samplerIndex;
} IvyModelAnimationChannel;

typedef enum IvyModelAnimationInterpolation {
  IVY_ANIMATION_INTERPOLATION_LINEAR = 0,
  IVY_ANIMATION_INTERPOLATION_STEP = 1,
  IVY_ANIMATION_INTERPOLATION_CUBICSPLINE = 2
} IvyModelAnimationInterpolation;

typedef struct IvyModelAnimationSampler {
  int32_t interpolationIndex;
  int32_t inputCount;
  float *inputs;
  int32_t outputCount;
  IvyV4 *outputs;
} IvyModelAnimationSampler;

typedef struct IvyModelAnimation {
  char name[256];
  float timeInMiliseconds;
  float timeStartInMiliseconds;
  float timeEndInMiliseconds;
  int32_t samplersIndexCount;
  int32_t *samplersIndices;
  int32_t channelsIndexCount;
  int32_t channelsIndices;
} IvyModelAnimation;

// TODO: move this elsewhere
typedef struct IvyModelPushConstant {
  IvyV4 baseColorFactor;
  IvyV4 emissiveFactor;
  IvyV4 diffuseFactor;
  IvyV4 specularFactor;
  float workflow;
  int32_t baseColorTexcoord;
  int32_t physicalDescriptionTexcoord;
  int32_t normalTexcoord;
  int32_t occlusionTexcoord;
  int32_t emissiveTexcoord;
  float metallicFactor;
  float roughnessFactor;
  float alphaMask;
  float alphaMaskCutoff;
} IvyModelPushConstant;

typedef struct IvyModelTexture {
  int32_t imageIndex;
} IvyModelTexture;

typedef struct IvyModel {
  IvyAnyMemoryAllocator ownerMemoryAllocator;
  char path[256];
  char directory[256];
  IvyGraphicsVertexBuffer *vertexBuffer;
  IvyGraphicsIndexBuffer *indexBuffer;
  int32_t activeAnimationIndex;
  int32_t rootNodeCount;
  IvyModelNode rootNodes;
  IvyGraphicsTexture *emptyTexture;
  int32_t imageCount;
  IvyGraphicsTexture **images;
  int32_t textureCount;
  IvyModelTexture *textures;
  int32_t materialCount;
  IvyModelMaterial *materials;
  int32_t meshCount;
  IvyModelMesh *meshes;
  int32_t primitiveCount;
  IvyModelPrimitive *primitives;
  int32_t skinCount;
  IvyModelSkin *skins;
  int32_t animationSamplerCount;
  IvyModelAnimationSampler *animationSamplers;
  int32_t animationChannelCount;
  IvyModelAnimationChannel *animationChannels;
  int32_t animationCount;
  IvyModelAnimation *animations;
} IvyModel;

#endif
