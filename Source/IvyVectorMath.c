#include "IvyVectorMath.h"
#include "IvyDeclarations.h"

#include <math.h>

#define IVY_PI 3.14159265358979323846

IVY_INTERNAL float ivySqrt(float value) {
  return sqrtf(value);
}

IVY_API float ivyDegToRad(float deg) {
  return deg / 180.0F * IVY_PI;
}

IVY_API void ivySetV3(float x, float y, float z, IvyV3 *out) {
  out->x = x;
  out->y = y;
  out->z = z;
}

IVY_API void ivyCopyV3(IvyV3 const *vector, IvyV3 *out) {
  out->x = vector->x;
  out->y = vector->y;
  out->z = vector->z;
}

IVY_API void ivyCopyV4(IvyV4 const *vector, IvyV4 *out) {
  out->x = vector->x;
  out->y = vector->y;
  out->z = vector->z;
  out->w = vector->w;
}

IVY_API void ivyCopyM4(IvyM4 const *matrix, IvyM4 *out) {
  IVY_MEMCPY(out, matrix, sizeof(IvyM4));
}

IVY_API void ivyIdentityM3(IvyM3 *matrix) {
  matrix->a[0][0] = 1.0F;
  matrix->a[0][1] = 0.0F;
  matrix->a[0][2] = 0.0F;
  matrix->a[1][0] = 0.0F;
  matrix->a[1][1] = 1.0F;
  matrix->a[1][2] = 0.0F;
  matrix->a[2][0] = 0.0F;
  matrix->a[2][1] = 0.0F;
  matrix->a[2][2] = 1.0F;
}

IVY_API void ivyIdentityM4(IvyM4 *matrix) {
  matrix->a[0][0] = 1.0F;
  matrix->a[0][1] = 0.0F;
  matrix->a[0][2] = 0.0F;
  matrix->a[0][3] = 0.0F;
  matrix->a[1][0] = 0.0F;
  matrix->a[1][1] = 1.0F;
  matrix->a[1][2] = 0.0F;
  matrix->a[1][3] = 0.0F;
  matrix->a[2][0] = 0.0F;
  matrix->a[2][1] = 0.0F;
  matrix->a[2][2] = 1.0F;
  matrix->a[2][3] = 0.0F;
  matrix->a[3][0] = 0.0F;
  matrix->a[3][1] = 0.0F;
  matrix->a[3][2] = 0.0F;
  matrix->a[3][3] = 1.0F;
}

IVY_API void ivyBroadcastV2(float value, IvyV2 *vector) {
  vector->x = value;
  vector->y = value;
}

IVY_API void ivyBroadcastV3(float value, IvyV3 *vector) {
  vector->x = value;
  vector->y = value;
  vector->z = value;
}

IVY_API void ivyBroadcastV4(float value, IvyV4 *vector) {
  vector->x = value;
  vector->y = value;
  vector->z = value;
  vector->w = value;
}

IVY_API float ivyGetMagnitudeOfV2(IvyV2 const *vector) {
  return ivySqrt(ivyDotV2ToV2(vector, vector));
}

IVY_API float ivyGetMagnitudeOfV3(IvyV3 const *vector) {
  return ivySqrt(ivyDotV3ToV3(vector, vector));
}

IVY_API float ivyGetMagnitudeOfV4(IvyV4 const *vector) {
  return ivySqrt(ivyDotV4ToV4(vector, vector));
}

IVY_API void ivyScaleV2(float value, IvyV2 *inOut) {
  inOut->x *= value;
  inOut->y *= value;
}

IVY_API void ivyScaleV3(float value, IvyV3 *inOut) {
  inOut->x *= value;
  inOut->y *= value;
  inOut->z *= value;
}

IVY_API void ivyScaleV4(float value, IvyV4 *inOut) {
  inOut->x *= value;
  inOut->y *= value;
  inOut->z *= value;
  inOut->w *= value;
}

IVY_API void ivyNormalizeV2(IvyV2 *vector) {
  ivyScaleV2(1 / ivyGetMagnitudeOfV2(vector), vector);
}

IVY_API void ivyNormalizeV3(IvyV3 *vector) {
  ivyScaleV3(1 / ivyGetMagnitudeOfV3(vector), vector);
}

IVY_API void ivyNormalizeV4(IvyV4 *vector) {
  ivyScaleV4(1 / ivyGetMagnitudeOfV4(vector), vector);
}

IVY_API void ivyZeroV2(IvyV2 *out) {
  ivyBroadcastV2(0.0F, out);
}

IVY_API void ivyZeroV3(IvyV3 *out) {
  ivyBroadcastV3(0.0F, out);
}

IVY_API void ivyZeroV4(IvyV4 *out) {
  ivyBroadcastV4(0.0F, out);
}

IVY_API void ivyInvM2(IvyM2 *out) {
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyInvM3(IvyM3 *out) {
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyInvM4(IvyM4 *out) {
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyCrossV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs, IvyV2 *out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyCrossV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs, IvyV3 *out) {
  IvyV3 lhsCopy;
  IvyV3 rhsCopy;

  ivyCopyV3(lhs, &lhsCopy);
  ivyCopyV3(rhs, &rhsCopy);

  out->x = lhsCopy.y * rhsCopy.z - lhsCopy.z * rhsCopy.y;
  out->y = lhsCopy.z * rhsCopy.x - lhsCopy.x * rhsCopy.z;
  out->z = lhsCopy.x * rhsCopy.y - lhsCopy.y * rhsCopy.x;
}

IVY_API void ivyCrossV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs, IvyV4 *out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API float ivyDotV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs) {
  return lhs->x * rhs->x + lhs->y * rhs->y;
}

IVY_API float ivyDotV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs) {
  return lhs->x * rhs->x + lhs->y * rhs->y + lhs->z * rhs->z;
}

IVY_API float ivyDotV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs) {
  return lhs->x * rhs->x + lhs->y * rhs->y + lhs->z * rhs->z + lhs->w * rhs->w;
}

IVY_API void ivyAddV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs, IvyV2 *out) {
  out->x = lhs->x + rhs->x;
  out->y = lhs->y + rhs->y;
}

IVY_API void ivyAddV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs, IvyV3 *out) {
  out->x = lhs->x + rhs->x;
  out->y = lhs->y + rhs->y;
  out->z = lhs->z + rhs->z;
}

IVY_API void ivyAddV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs, IvyV4 *out) {
  out->x = lhs->x + rhs->x;
  out->y = lhs->y + rhs->y;
  out->z = lhs->z + rhs->z;
  out->w = lhs->w + rhs->w;
}

IVY_API void ivySubV2ToV2(IvyV2 const *lhs, IvyV2 const *rhs, IvyV2 *out) {
  out->x = lhs->x - rhs->x;
  out->y = lhs->y - rhs->y;
}

IVY_API void ivySubV3ToV3(IvyV3 const *lhs, IvyV3 const *rhs, IvyV3 *out) {
  out->x = lhs->x - rhs->x;
  out->y = lhs->y - rhs->y;
  out->z = lhs->z - rhs->z;
}

IVY_API void ivySubV4ToV4(IvyV4 const *lhs, IvyV4 const *rhs, IvyV4 *out) {
  out->x = lhs->x - rhs->x;
  out->y = lhs->y - rhs->y;
  out->z = lhs->z - rhs->z;
  out->w = lhs->w - rhs->w;
}

IVY_API void ivyAddM2ToM2(IvyM2 const *lhs, IvyM2 const *rhs, IvyM2 *out) {
  out->a[0][0] = lhs->a[0][0] + rhs->a[0][0];
  out->a[0][1] = lhs->a[0][1] + rhs->a[0][1];
  out->a[1][0] = lhs->a[1][0] + rhs->a[1][0];
  out->a[1][1] = lhs->a[1][1] + rhs->a[1][1];
}

IVY_API void ivyAddM3ToM3(IvyM3 const *lhs, IvyM3 const *rhs, IvyM3 *out) {
  out->a[0][0] = lhs->a[0][0] + rhs->a[0][0];
  out->a[0][1] = lhs->a[0][1] + rhs->a[0][1];
  out->a[0][2] = lhs->a[0][2] + rhs->a[0][2];
  out->a[1][0] = lhs->a[1][0] + rhs->a[1][0];
  out->a[1][1] = lhs->a[1][1] + rhs->a[1][1];
  out->a[1][2] = lhs->a[1][2] + rhs->a[1][2];
  out->a[2][0] = lhs->a[2][0] + rhs->a[2][0];
  out->a[2][1] = lhs->a[2][1] + rhs->a[2][1];
  out->a[2][2] = lhs->a[2][2] + rhs->a[2][2];
}

IVY_API void ivyAddM4ToM4(IvyM4 const *lhs, IvyM4 const *rhs, IvyM4 *out) {
  out->a[0][0] = lhs->a[0][0] + rhs->a[0][0];
  out->a[0][1] = lhs->a[0][1] + rhs->a[0][1];
  out->a[0][2] = lhs->a[0][2] + rhs->a[0][2];
  out->a[0][3] = lhs->a[0][3] + rhs->a[0][3];
  out->a[1][0] = lhs->a[1][0] + rhs->a[1][0];
  out->a[1][1] = lhs->a[1][1] + rhs->a[1][1];
  out->a[1][2] = lhs->a[1][2] + rhs->a[1][2];
  out->a[1][3] = lhs->a[1][3] + rhs->a[1][3];
  out->a[2][0] = lhs->a[2][0] + rhs->a[2][0];
  out->a[2][1] = lhs->a[2][1] + rhs->a[2][1];
  out->a[2][2] = lhs->a[2][2] + rhs->a[2][2];
  out->a[2][3] = lhs->a[2][3] + rhs->a[2][3];
  out->a[3][0] = lhs->a[3][0] + rhs->a[3][0];
  out->a[3][1] = lhs->a[3][1] + rhs->a[3][1];
  out->a[3][2] = lhs->a[3][2] + rhs->a[3][2];
  out->a[3][3] = lhs->a[3][3] + rhs->a[3][3];
}

IVY_API void ivyMulM2ToM2(IvyM2 const *lhs, IvyM2 const *rhs, IvyM2 *out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyMulM3ToM3(IvyM3 const *lhs, IvyM3 const *rhs, IvyM3 *out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyMulM4ToM4(IvyM4 const *lhs, IvyM4 const *rhs, IvyM4 *out) {
  /* clang-format off */
  float const
    a00 = lhs->a[0][0], a01 = lhs->a[0][1], a02 = lhs->a[0][2], a03 = lhs->a[0][3], // NOLINT
    a10 = lhs->a[1][0], a11 = lhs->a[1][1], a12 = lhs->a[1][2], a13 = lhs->a[1][3],
    a20 = lhs->a[2][0], a21 = lhs->a[2][1], a22 = lhs->a[2][2], a23 = lhs->a[2][3],
    a30 = lhs->a[3][0], a31 = lhs->a[3][1], a32 = lhs->a[3][2], a33 = lhs->a[3][3],

    b00 = rhs->a[0][0], b01 = rhs->a[0][1], b02 = rhs->a[0][2], b03 = rhs->a[0][3],
    b10 = rhs->a[1][0], b11 = rhs->a[1][1], b12 = rhs->a[1][2], b13 = rhs->a[1][3],
    b20 = rhs->a[2][0], b21 = rhs->a[2][1], b22 = rhs->a[2][2], b23 = rhs->a[2][3],
    b30 = rhs->a[3][0], b31 = rhs->a[3][1], b32 = rhs->a[3][2], b33 = rhs->a[3][3];
  /* clang-format on */

  out->a[0][0] = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
  out->a[0][1] = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
  out->a[0][2] = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
  out->a[0][3] = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;
  out->a[1][0] = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
  out->a[1][1] = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
  out->a[1][2] = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
  out->a[1][3] = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;
  out->a[2][0] = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
  out->a[2][1] = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
  out->a[2][2] = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
  out->a[2][3] = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;
  out->a[3][0] = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
  out->a[3][1] = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
  out->a[3][2] = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
  out->a[3][3] = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;
}

IVY_API void ivyMakeRotateM4(float angle, IvyV3 const *axis, IvyM4 *out) {
  IVY_UNUSED(angle);
  IVY_UNUSED(axis);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyTranslateM4(IvyV3 const *offset, IvyM4 *inOut) {
  IvyV4 v0;
  IvyV4 v1;
  IvyV4 v2;

  v0.x = inOut->a[0][0];
  v0.y = inOut->a[0][1];
  v0.z = inOut->a[0][2];
  v0.w = inOut->a[0][3];

  v1.x = inOut->a[1][0];
  v1.y = inOut->a[1][1];
  v1.z = inOut->a[1][2];
  v1.w = inOut->a[1][3];

  v2.x = inOut->a[2][0];
  v2.y = inOut->a[2][1];
  v2.z = inOut->a[2][2];
  v2.w = inOut->a[2][3];

  ivyScaleV4(offset->x, &v0);
  ivyScaleV4(offset->y, &v1);
  ivyScaleV4(offset->z, &v2);

  inOut->a[3][0] += v0.x;
  inOut->a[3][0] += v0.y;
  inOut->a[3][0] += v0.z;

  inOut->a[3][0] += v1.x;
  inOut->a[3][0] += v1.y;
  inOut->a[3][0] += v1.z;

  inOut->a[3][0] += v2.x;
  inOut->a[3][0] += v2.y;
  inOut->a[3][0] += v2.z;
}

IVY_API void ivyCreateOrthographicM4(float left, float right, float bottom,
    float top, float near, float far, IvyM4 *out) {
  IVY_UNUSED(left);
  IVY_UNUSED(right);
  IVY_UNUSED(bottom);
  IVY_UNUSED(top);
  IVY_UNUSED(near);
  IVY_UNUSED(far);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_INTERNAL float ivyTan(float value) {
  return tanf(value);
}

IVY_API void ivyCreatePerspectiveM4(float fov, float ratio, float near,
    float far, IvyM4 *out) {
  float const focalLength = 1.0F / ivyTan(fov * 0.5F);
  float const inverseOfFarNear = 1.0F / (far - near);

  out->a[0][0] = focalLength / ratio;
  out->a[0][1] = 0.0F;
  out->a[0][2] = 0.0F;
  out->a[0][3] = 0.0F;

  out->a[1][0] = 0.0F;
  out->a[1][1] = focalLength;
  out->a[1][2] = 0.0F;
  out->a[1][3] = 0.0F;

  out->a[2][0] = 0.0F;
  out->a[2][1] = 0.0F;
  out->a[2][2] = -far * inverseOfFarNear;
  out->a[2][3] = -1.0F;

  out->a[3][0] = 0.0F;
  out->a[3][1] = 0.0F;
  out->a[3][2] = -far * near * inverseOfFarNear;
  out->a[3][3] = 0.0F;
}

IVY_API void ivyCreateLookM4(IvyV3 const *eye, IvyV3 const *direction,
    IvyV3 const *upDirection, IvyM4 *out) {
  IvyV3 atPoint;

  ivyAddV3ToV3(eye, direction, &atPoint);
  ivyCreateLookAtM4(eye, &atPoint, upDirection, out);
}

IVY_API void ivyCreateLookAtM4(IvyV3 const *eye, IvyV3 const *atPoint,
    IvyV3 const *upDirection, IvyM4 *out) {
  IvyV3 xAxis;
  IvyV3 yAxis;
  IvyV3 zAxis;

  ivySubV3ToV3(atPoint, eye, &zAxis);
  ivyNormalizeV3(&zAxis);

  ivyCrossV3ToV3(&zAxis, upDirection, &xAxis);
  ivyNormalizeV3(&xAxis);

  ivyCrossV3ToV3(&xAxis, &zAxis, &yAxis);
  ivyNormalizeV3(&yAxis);

  out->a[0][0] = xAxis.x;
  out->a[0][1] = yAxis.x;
  out->a[0][2] = zAxis.x;
  out->a[0][3] = 0.0F;

  out->a[1][0] = xAxis.y;
  out->a[1][1] = yAxis.y;
  out->a[1][2] = zAxis.y;
  out->a[1][3] = 0.0F;

  out->a[2][0] = xAxis.z;
  out->a[2][1] = yAxis.z;
  out->a[2][2] = zAxis.z;
  out->a[2][3] = 0.0F;

  out->a[3][0] = -ivyDotV3ToV3(&xAxis, eye);
  out->a[3][1] = -ivyDotV3ToV3(&yAxis, eye);
  out->a[3][2] = -ivyDotV3ToV3(&zAxis, eye);
  out->a[3][3] = 0.0F;
}

IVY_API void ivyCreateDirectionV3(float pitch, float yaw,
    IvyV3 const *upDirection, IvyV3 const *out) {
  IVY_UNUSED(pitch);
  IVY_UNUSED(yaw);
  IVY_UNUSED(upDirection);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyRotateM4(IvyM4 *matrix, float angle, IvyV3 const *axis,
    IvyM4 *out) {
  IVY_UNUSED(matrix);
  IVY_UNUSED(angle);
  IVY_UNUSED(axis);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyGetDistanceBetweenV2AndV2(IvyV2 const *source,
    IvyV2 const *destination) {
  IVY_UNUSED(source);
  IVY_UNUSED(destination);
  IVY_TODO();
}
IVY_API void ivyGetDistanceBetweenV3AndV3(IvyV3 const *source,
    IvyV3 const *destination) {
  IVY_UNUSED(source);
  IVY_UNUSED(destination);
  IVY_TODO();
}

IVY_API void ivyGetDistanceBetweenV4AndV4(IvyV4 const *source,
    IvyV4 const *destination) {
  IVY_UNUSED(source);
  IVY_UNUSED(destination);
  IVY_TODO();
}

IVY_API void ivyMixV3(IvyV3 const *source, IvyV3 const *destination,
    float weight, IvyV3 const *out) {
  IVY_UNUSED(source);
  IVY_UNUSED(destination);
  IVY_UNUSED(weight);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyMixV4(IvyV4 const *source, IvyV4 const *destination,
    float weight, IvyV4 const *out) {
  IVY_UNUSED(source);
  IVY_UNUSED(destination);
  IVY_UNUSED(weight);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyQ4AsM4(IvyQ4 const *quaternion, IvyM3 *out) {
  IVY_UNUSED(quaternion);
  IVY_UNUSED(out);
  IVY_TODO();
}

IVY_API void ivyScaleM4ByV3(IvyM4 const *matrix, IvyV3 const *scale,
    IvyM4 *out) {
  IVY_UNUSED(matrix);
  IVY_UNUSED(scale);
  IVY_UNUSED(out);
  IVY_TODO();
}
