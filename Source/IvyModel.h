#ifndef IVY_MODEL_H
#define IVY_MODEL_H

#include "IvyGraphicsMemoryAllocator.h"
#include "IvyVectorMath.h"

typedef enum IvyModelMaterialAlphaMode {
  IVY_MODEL_MATERIAL_ALPHA_MODE_OPAQUE = 0,
  IVY_MODEL_MATERIAL_ALPHA_MODE_MASK = 1,
  IVY_MODEL_MATERIAL_ALPHA_MODE_BLEND = 2,
} IvyModelMaterialAlphaMode;

typedef struct IvyBoundingBox {
  IvyV3 min;
  IvyV3 max;
  IvyBool valid;
} IvyBoundingBox;

typedef struct IvyModelMaterial {
  IvyModelMaterialAlphaMode alphaMode;
} IvyModelMaterial;

typedef struct IvyModel {
  int empty;
} IvyModel;

#endif
