#ifndef IVY_VECTOR_MATH_H
#define IVY_VECTOR_MATH_H

#include "IvyDeclarations.h"

typedef struct IvyV2 {
  float x;
  float y;
} IvyV2;

typedef struct IvyV3 {
  float x;
  float y;
  float z;
} IvyV3;

typedef struct IvyV4 {
  float x;
  float y;
  float z;
  float w;
} IvyV4;

typedef struct IvyQ4 {
  float x;
  float y;
  float z;
  float w;
} IvyQ4;

typedef struct IvyM2 {
  float a[2][2];
} IvyM2;

typedef struct IvyM3 {
  float a[3][3];
} IvyM3;

typedef struct IvyM4 {
  float a[4][4];
} IvyM4;

IVY_API float ivyDegToRad(float deg);

IVY_API void ivySetV3(float x, float y, float z, IvyV3 *out);

// TODO(samuel): inline versions

IVY_API void ivyCopyV3(IvyV3 const *vector, IvyV3 *out);
IVY_API void ivyCopyV4(IvyV4 const *vector, IvyV4 *out);
IVY_API void ivyCopyM4(IvyM4 const *matrix, IvyM4 *out);

IVY_API void ivyIdentityM3(IvyM3 *matrix);
IVY_API void ivyIdentityM4(IvyM4 *matrix);

IVY_API void ivyBroadcastV2(float value, IvyV2 *vector);
IVY_API void ivyBroadcastV3(float value, IvyV3 *vector);
IVY_API void ivyBroadcastV4(float value, IvyV4 *vector);

IVY_API float ivyGetMagnitudeOfV2(IvyV2 const *vector);
IVY_API float ivyGetMagnitudeOfV3(IvyV3 const *vector);
IVY_API float ivyGetMagnitudeOfV4(IvyV4 const *vector);

IVY_API void ivyScaleV2(float value, IvyV2 *inOut);
IVY_API void ivyScaleV3(float value, IvyV3 *inOut);
IVY_API void ivyScaleV4(float value, IvyV4 *inOut);

IVY_API void ivyNormalizeV2(IvyV2 *vector);
IVY_API void ivyNormalizeV3(IvyV3 *vector);
IVY_API void ivyNormalizeV4(IvyV4 *vector);

IVY_API void ivyZeroV2(IvyV2 *out);
IVY_API void ivyZeroV3(IvyV3 *out);
IVY_API void ivyZeroV4(IvyV4 *out);

IVY_API void ivyInvM2(IvyM2 *inOut);
IVY_API void ivyInvM3(IvyM3 *inOut);
IVY_API void ivyInvM4(IvyM4 *inOut);

IVY_API void ivyCrossV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs, IvyV2 *out);
IVY_API void ivyCrossV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs, IvyV3 *out);
IVY_API void ivyCrossV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs, IvyV4 *out);

IVY_API float ivyDotV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs);
IVY_API float ivyDotV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs);
IVY_API float ivyDotV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs);

IVY_API void ivyAddV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs, IvyV2 *out);
IVY_API void ivyAddV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs, IvyV3 *out);
IVY_API void ivyAddV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs, IvyV4 *out);

IVY_API void ivySubV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs, IvyV2 *out);
IVY_API void ivySubV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs, IvyV3 *out);
IVY_API void ivySubV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs, IvyV4 *out);

IVY_API void ivyAddM2ToM2(IvyM2 const *lhs, IvyM2 const *rhs, IvyM2 *out);
IVY_API void ivyAddM3ToM3(IvyM3 const *lhs, IvyM3 const *rhs, IvyM3 *out);
IVY_API void ivyAddM4ToM4(IvyM4 const *lhs, IvyM4 const *rhs, IvyM4 *out);

IVY_API void ivyMulM2ToM2(IvyM2 const *lhs, IvyM2 const *rhs, IvyM2 *out);
IVY_API void ivyMulM3ToM3(IvyM3 const *lhs, IvyM3 const *rhs, IvyM3 *out);
IVY_API void ivyMulM4ToM4(IvyM4 const *lhs, IvyM4 const *rhs, IvyM4 *out);

IVY_API void ivyMakeRotateM4(float angle, IvyV3 const *axis, IvyM4 *out);
IVY_API void ivyTranslateM4(IvyV3 const *offset, IvyM4 *inOut);

IVY_API void ivyCreateOrthographicM4(float left, float right, float bottom,
    float top, float near, float far, IvyM4 *out);

IVY_API void ivyCreatePerspectiveM4(float fov, float ratio, float near,
    float far, IvyM4 *out);

IVY_API void ivyCreateLookM4(IvyV3 const *eye, IvyV3 const *direction,
    IvyV3 const *upDirection, IvyM4 *out);
IVY_API void ivyCreateLookAtM4(IvyV3 const *eye, IvyV3 const *atPoint,
    IvyV3 const *upDirection, IvyM4 *out);
IVY_API void ivyCreateDirectionV3(float pitch, float yaw, IvyV3 const *up,
    IvyV3 const *out);

IVY_API void ivyRotateM4(IvyM4 *matrix, float angle, IvyV3 const *axis,
    IvyM4 *out);

IVY_API void ivyGetDistanceBetweenV2AndV2(IvyV2 const *source,
    IvyV2 const *destination);
IVY_API void ivyGetDistanceBetweenV3AndV3(IvyV3 const *source,
    IvyV3 const *destination);
IVY_API void ivyGetDistanceBetweenV4AndV4(IvyV4 const *source,
    IvyV4 const *destination);

IVY_API void ivyMixV3(IvyV3 const *source, IvyV3 const *destination,
    float weight, IvyV3 const *out);
IVY_API void ivyMixV4(IvyV4 const *source, IvyV4 const *destination,
    float weight, IvyV4 const *out);
IVY_API void ivyQ4AsM4(IvyQ4 const *quaternion, IvyM3 *out);
IVY_API void ivyScaleM4ByV3(IvyM4 const *matrix, IvyV3 const *scale,
    IvyM4 *out);

IVY_API void ivyPrintM4(IvyM4 const *matrix);

#endif
