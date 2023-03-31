#include "IvyModel.h"

#include <cgltf.h>

#include <stdio.h>

#include "IvyGraphicsTexture.h"
#include "IvyRenderer.h"

IVY_INTERNAL void const *ivyResolveGLTFAccessor(cgltf_accessor *accessor) {
  cgltf_buffer_view const *view = accessor->buffer_view;
  return ((uint8_t *)view->data) + accessor->offset + view->offset;
}

// https://www.redhat.com/en/blog/trouble-snprintf
IVY_INTERNAL IvyCode ivyFixModelURI(char const *assetDirectory,
    char const *uri, uint64_t maxFixedURISize, char *fixedURI) {
  int written;

  written = snprintf(fixedURI, maxFixedURISize, "%s/%s", assetDirectory, uri);
  // NOTE: if written is negative, then it just errors out, so there will be no
  // int wrapping
  if (written < 0 || (uint64_t)written >= maxFixedURISize) {
    return IVY_ERROR_UNKNOWN;
  }

  return IVY_OK;
}

IVY_INTERNAL IvyCode ivyLoadModelImages(IvyAnyMemoryAllocator allocator,
    IvyRenderer *renderer, cgltf_data *cgltfData, uint32_t *imageCount,
    IvyGraphicsTexture ***images) {
  IvyCode ivyCode;
  uint32_t imageIndex;
  uint32_t currentImageCount;
  IvyGraphicsTexture **currentImages = NULL;

  currentImageCount = cgltfData->images_count;
  currentImages =
      ivyAllocateMemory(allocator, currentImageCount * sizeof(*currentImages));
  if (!currentImages) {
    ivyCode = IVY_ERROR_NO_MEMORY;
    goto error;
  }

  IVY_MEMSET(currentImages, 0, currentImageCount * sizeof(*currentImages));

  for (imageIndex = 0; imageIndex < currentImageCount; ++imageIndex) {
    char fixedURI[256];
    cgltf_image const *cgltfImage = &cgltfData->images[imageIndex];
    IvyGraphicsTexture **image = &currentImages[imageIndex];

    ivyCode = ivyFixModelURI("../Resources/", cgltfImage->uri,
        sizeof(fixedURI), fixedURI);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }

    ivyCode =
        ivyCreateGraphicsTextureFromFile(allocator, renderer, fixedURI, image);
    IVY_ASSERT(!ivyCode);
    if (ivyCode) {
      goto error;
    }
  }

  *imageCount = currentImageCount;
  *images = currentImages;

  return IVY_OK;

error:
  if (currentImages) {
    for (imageIndex = 0; imageIndex < currentImageCount; ++imageIndex) {
      ivyDestroyGraphicsTexture(allocator, renderer,
          currentImages[imageIndex]);
    }

    ivyFreeMemory(allocator, currentImages);
  }

  *imageCount = 0;
  *images = NULL;
  return ivyCode;
}

IVY_INTERNAL IvyCode ivyLoadModelTextures(IvyAnyMemoryAllocator allocator,
    cgltf_data *cgltfData, uint32_t *textureCount,
    IvyModelTexture **textures) {

  uint32_t textureIndex;
  uint32_t currentTextureCount;
  IvyModelTexture *currentTextures;

  currentTextureCount = cgltfData->textures_count;
  currentTextures = ivyAllocateMemory(allocator,
      currentTextureCount * sizeof(*currentTextures));
  if (!currentTextures) {
    *textureCount = 0;
    *textures = NULL;
    return IVY_ERROR_NO_MEMORY;
  }

  for (textureIndex = 0; textureIndex < currentTextureCount; ++textureIndex) {
    cgltf_texture const *cgltfTexture = &cgltfData->textures[textureIndex];
    uint32_t imageIndex = (uint32_t)(cgltfTexture->image - cgltfData->images);
    currentTextures[textureIndex].imageIndex = imageIndex;
  }

  *textureCount = currentTextureCount;
  *textures = currentTextures;

  return IVY_OK;
}

IVY_INTERNAL IvyCode ivyLoadModelMaterials(IvyAnyMemoryAllocator allocator,
    cgltf_data *cgltfData, uint32_t *materialCount,
    IvyModelMaterial **materials) {
  uint32_t materialIndex;
  uint32_t currentMaterialCount;
  IvyModelMaterial *currentMaterials;

  currentMaterialCount = cgltfData->materials_count;
  currentMaterials = ivyAllocateMemory(allocator,
      currentMaterialCount * sizeof(*currentMaterials));
  if (!currentMaterials) {
    *materialCount = 0;
    *materials = NULL;
    return IVY_ERROR_NO_MEMORY;
  }

  for (materialIndex = 0; materialIndex < currentMaterialCount;
       ++materialIndex) {
    cgltf_material const *cgltfMaterial = &cgltfData->materials[materialIndex];
    IvyModelMaterial *material = &currentMaterials[materialIndex];

    material->isDoubleSided = cgltfMaterial->double_sided;
    material->alphaCutoff = cgltfMaterial->alpha_cutoff;
    material->emissiveFactor.x = 1.0F;
    material->emissiveFactor.y = 1.0F;
    material->emissiveFactor.z = 1.0F;
    material->emissiveFactor.w = 1.0F;
    material->baseColorTexcoord = -1;
    material->normalTexcoord = -1;
    material->emissiveTexcoord = -1;
    material->baseColorTexcoord = -1;
    material->baseColorTextureIndex = -1;
    material->metallicRoughnessTextureIndex = -1;
    material->metallicRoughnessTexcoord = -1;
    material->specularGlossinessTextureIndex = -1;
    material->specularGlossinessTexcoord = -1;
    material->roughnessFactor = 1.0F;
    material->metallicFactor = 1.0F;
    material->baseColorFactor.x = 1.0F;
    material->baseColorFactor.y = 1.0F;
    material->baseColorFactor.z = 1.0F;
    material->baseColorFactor.w = 1.0F;

    material->enableMetallicRoughness =
        cgltfMaterial->has_pbr_metallic_roughness;
    material->enableSpecularGlossiness =
        cgltfMaterial->has_pbr_specular_glossiness;

    if (material->enableMetallicRoughness) {
      cgltf_pbr_metallic_roughness const *cgltfPBRMetallicRoughness =
          &cgltfMaterial->pbr_metallic_roughness;
      cgltf_texture_view const *cgltfBaseColorTextureView =
          &cgltfPBRMetallicRoughness->base_color_texture;
      cgltf_texture_view const *cgltfMetallicRoughnessTextureView =
          &cgltfPBRMetallicRoughness->metallic_roughness_texture;

      if (cgltfBaseColorTextureView->texture) {
        cgltf_texture const *texture = cgltfBaseColorTextureView->texture;
        int const texcoord = cgltfBaseColorTextureView->texcoord;

        material->baseColorTextureIndex =
            (uint32_t)(texture - cgltfData->textures);
        material->baseColorTexcoord = texcoord;
      } else {
        material->baseColorTextureIndex = -1;
        material->baseColorTexcoord = -1;
      }

      if (cgltfMetallicRoughnessTextureView->texture) {
        cgltf_texture const *texture =
            cgltfMetallicRoughnessTextureView->texture;
        int const texcoord = cgltfMetallicRoughnessTextureView->texcoord;

        material->metallicRoughnessTextureIndex =
            (uint32_t)(texture - cgltfData->textures);
        material->metallicRoughnessTexcoord = texcoord;
      } else {
        material->metallicRoughnessTextureIndex = -1;
        material->metallicRoughnessTexcoord = -1;
      }

      material->roughnessFactor = cgltfPBRMetallicRoughness->roughness_factor;
      material->metallicFactor = cgltfPBRMetallicRoughness->metallic_factor;
      material->baseColorFactor.x =
          cgltfPBRMetallicRoughness->base_color_factor[0];
      material->baseColorFactor.y =
          cgltfPBRMetallicRoughness->base_color_factor[1];
      material->baseColorFactor.z =
          cgltfPBRMetallicRoughness->base_color_factor[2];
      material->baseColorFactor.w =
          cgltfPBRMetallicRoughness->base_color_factor[3];
    }

    if (cgltfMaterial->normal_texture.texture) {
      cgltf_texture_view const *view = &cgltfMaterial->normal_texture;
      material->normalTextureIndex =
          (uint32_t)(view->texture - cgltfData->textures);
      material->normalTexcoord = view->texcoord;
    } else {
      material->normalTextureIndex = -1;
      material->normalTexcoord = -1;
    }

    if (cgltfMaterial->emissive_texture.texture) {
      cgltf_texture_view const *view = &cgltfMaterial->emissive_texture;
      material->emissiveTextureIndex =
          (uint32_t)(view->texture - cgltfData->textures);
      material->emissiveTexcoord = view->texcoord;
    } else {
      material->emissiveTextureIndex = -1;
      material->emissiveTexcoord = -1;
    }

    if (cgltfMaterial->occlusion_texture.texture) {
      cgltf_texture_view const *view = &cgltfMaterial->occlusion_texture;
      material->occlusionTextureIndex =
          (uint32_t)(view->texture - cgltfData->textures);
      material->occlusionTexcoord = view->texcoord;
    } else {
      material->occlusionTextureIndex = -1;
      material->occlusionTexcoord = -1;
    }

    material->alphaMode = (float)cgltfMaterial->alpha_mode;
    material->alphaCutoff = cgltfMaterial->alpha_cutoff;

    if (material->enableSpecularGlossiness) {
      cgltf_pbr_specular_glossiness const *cgltfSpecularGlossiness =
          &cgltfMaterial->pbr_specular_glossiness;
      cgltf_texture_view const *cgltfSpecularGlossinessTextureView =
          &cgltfSpecularGlossiness->specular_glossiness_texture;
      cgltf_texture_view const *cgltfDiffuseTextureView =
          &cgltfSpecularGlossiness->diffuse_texture;

      if (cgltfSpecularGlossinessTextureView->texture) {
        cgltf_texture const *texture =
            cgltfSpecularGlossinessTextureView->texture;
        material->specularGlossinessTextureIndex =
            (uint32_t)(texture - cgltfData->textures);
        material->specularGlossinessTexcoord =
            cgltfDiffuseTextureView->texcoord;
      } else {
        material->specularGlossinessTextureIndex = -1;
        material->specularGlossinessTexcoord = -1;
      }

      if (cgltfDiffuseTextureView->texture) {
        cgltf_texture const *texture = cgltfDiffuseTextureView->texture;
        material->diffuseTextureIndex =
            (uint32_t)(texture - cgltfData->textures);
      } else {
        material->diffuseTextureIndex = -1;
      }

      material->diffuseFactor.x = cgltfSpecularGlossiness->diffuse_factor[0];
      material->diffuseFactor.y = cgltfSpecularGlossiness->diffuse_factor[1];
      material->diffuseFactor.z = cgltfSpecularGlossiness->diffuse_factor[2];
      material->diffuseFactor.w = cgltfSpecularGlossiness->diffuse_factor[3];

      material->specularFactor.x = cgltfSpecularGlossiness->specular_factor[0];
      material->specularFactor.y = cgltfSpecularGlossiness->specular_factor[1];
      material->specularFactor.z = cgltfSpecularGlossiness->specular_factor[2];
      material->specularFactor.w = 1.0F;
    }
  }

  return IVY_OK;
}
