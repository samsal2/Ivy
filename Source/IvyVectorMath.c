#include "IvyVectorMath.h"
#include "IvyDeclarations.h"

#include <math.h>

static float ivySqrt(float value) { return sqrtf(value); }

void ivyCopyV3(IvyV3 vector, IvyV3 out) {
  out[0] = vector[0];
  out[1] = vector[1];
  out[2] = vector[2];
}

void ivyCopyV4(IvyV4 vector, IvyV4 out) {
  out[0] = vector[0];
  out[1] = vector[1];
  out[2] = vector[2];
  out[4] = vector[4];
}

void ivyIdentityM3(IvyM3 matrix) {
  matrix[0][0] = 1.0F;
  matrix[0][1] = 0.0F;
  matrix[0][2] = 0.0F;
  matrix[1][0] = 0.0F;
  matrix[1][1] = 1.0F;
  matrix[1][2] = 0.0F;
  matrix[2][0] = 0.0F;
  matrix[2][1] = 0.0F;
  matrix[2][2] = 1.0F;
}

void ivyIdentityM4(IvyM4 matrix) {
  matrix[0][0] = 1.0F;
  matrix[0][1] = 0.0F;
  matrix[0][2] = 0.0F;
  matrix[0][3] = 0.0F;
  matrix[1][0] = 0.0F;
  matrix[1][1] = 1.0F;
  matrix[1][2] = 0.0F;
  matrix[1][3] = 0.0F;
  matrix[2][0] = 0.0F;
  matrix[2][1] = 0.0F;
  matrix[2][2] = 1.0F;
  matrix[2][3] = 0.0F;
  matrix[3][0] = 0.0F;
  matrix[3][1] = 0.0F;
  matrix[3][2] = 0.0F;
  matrix[3][3] = 1.0F;
}

void ivyBroadcastV2(float value, IvyV2 vector) {
  vector[0] = value;
  vector[1] = value;
}

void ivyBroadcastV3(float value, IvyV3 vector) {
  vector[0] = value;
  vector[1] = value;
  vector[2] = value;
}

void ivyBroadcastV4(float value, IvyV4 vector) {
  vector[0] = value;
  vector[1] = value;
  vector[2] = value;
  vector[3] = value;
}

float ivyGetMagnitudeOfV2(IvyV2 vector) {
  return ivySqrt(ivyDotV2ToV2(vector, vector));
}

float ivyGetMagnitudeOfV3(IvyV3 vector) {
  return ivySqrt(ivyDotV3ToV3(vector, vector));
}

float ivyGetMagnitudeOfV4(IvyV4 vector) {
  return ivySqrt(ivyDotV4ToV4(vector, vector));
}

void ivyScaleV2(float value, IvyV2 inOut) {
  inOut[0] *= value;
  inOut[1] *= value;
}

void ivyScaleV3(float value, IvyV3 inOut) {
  inOut[0] *= value;
  inOut[1] *= value;
  inOut[2] *= value;
}

void ivyScaleV4(float value, IvyV4 inOut) {
  inOut[0] *= value;
  inOut[1] *= value;
  inOut[2] *= value;
  inOut[3] *= value;
}

void ivyNormalizeV2(IvyV2 vector) {
  ivyScaleV2(1 / ivyGetMagnitudeOfV2(vector), vector);
}

void ivyNormalizeV3(IvyV3 vector) {
  ivyScaleV3(1 / ivyGetMagnitudeOfV3(vector), vector);
}

void ivyNormalizeV4(IvyV4 vector) {
  ivyScaleV4(1 / ivyGetMagnitudeOfV4(vector), vector);
}

void ivyZeroV2(IvyV2 out) { ivyBroadcastV2(0.0F, out); }

void ivyZeroV3(IvyV3 out) { ivyBroadcastV3(0.0F, out); }

void ivyZeroV4(IvyV4 out) { ivyBroadcastV4(0.0F, out); }

void ivyInvM2(IvyM2 out);
void ivyInvM3(IvyM3 out);
void ivyInvM4(IvyM4 out);

void ivyCrossV2ToV2(IvyV2 lhs, IvyV2 rhs, IvyV2 out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyCrossV3ToV3(IvyV3 lhs, IvyV3 rhs, IvyV3 out) {
  IvyV3 lhsCopy, rhsCopy;

  ivyCopyV3(lhs, lhsCopy);
  ivyCopyV3(rhs, rhsCopy);

  out[0] = lhsCopy[1] * rhsCopy[2] - lhsCopy[2] * rhsCopy[1];
  out[1] = lhsCopy[2] * rhsCopy[0] - lhsCopy[0] * rhsCopy[2];
  out[2] = lhsCopy[0] * rhsCopy[1] - lhsCopy[1] * rhsCopy[0];
}

void ivyCrossV4ToV4(IvyV4 lhs, IvyV4 rhs, IvyV4 out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

float ivyDotV2ToV2(IvyV2 lhs, IvyV2 rhs) {
  return lhs[0] * rhs[0] + lhs[1] * rhs[1];
}

float ivyDotV3ToV3(IvyV3 lhs, IvyV3 rhs) {
  return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

float ivyDotV4ToV4(IvyV4 lhs, IvyV4 rhs) {
  return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3] * rhs[3];
}

void ivyAddV2ToV2(IvyV2 lhs, IvyV2 rhs, IvyV2 out) {
  out[0] = lhs[0] + rhs[0];
  out[1] = lhs[1] + rhs[1];
}

void ivyAddV3ToV3(IvyV3 lhs, IvyV3 rhs, IvyV3 out) {
  out[0] = lhs[0] + rhs[0];
  out[1] = lhs[1] + rhs[1];
  out[2] = lhs[2] + rhs[2];
}

void ivyAddV4ToV4(IvyV4 lhs, IvyV4 rhs, IvyV4 out) {
  out[0] = lhs[0] + rhs[0];
  out[1] = lhs[1] + rhs[1];
  out[2] = lhs[2] + rhs[2];
  out[3] = lhs[3] + rhs[3];
}

void ivySubV2ToV2(IvyV2 lhs, IvyV2 rhs, IvyV2 out) {
  out[0] = lhs[0] - rhs[0];
  out[1] = lhs[1] - rhs[1];
}

void ivySubV3ToV3(IvyV3 lhs, IvyV3 rhs, IvyV3 out) {
  out[0] = lhs[0] - rhs[0];
  out[1] = lhs[1] - rhs[1];
  out[2] = lhs[2] - rhs[2];
}

void ivySubV4ToV4(IvyV4 lhs, IvyV4 rhs, IvyV4 out) {
  out[0] = lhs[0] - rhs[0];
  out[1] = lhs[1] - rhs[1];
  out[2] = lhs[2] - rhs[2];
  out[3] = lhs[3] - rhs[3];
}

void ivyAddM2ToM2(IvyM2 lhs, IvyM2 rhs, IvyM2 out) {
  ivyAddV2ToV2(lhs[0], rhs[0], out[0]);
  ivyAddV2ToV2(lhs[1], rhs[1], out[1]);
}

void ivyAddM3ToM3(IvyM3 lhs, IvyM3 rhs, IvyM3 out) {
  ivyAddV3ToV3(lhs[0], rhs[0], out[0]);
  ivyAddV3ToV3(lhs[1], rhs[1], out[1]);
  ivyAddV3ToV3(lhs[2], rhs[2], out[2]);
}

void ivyAddM4ToM4(IvyM4 lhs, IvyM4 rhs, IvyM4 out) {
  ivyAddV4ToV4(lhs[0], rhs[0], out[0]);
  ivyAddV4ToV4(lhs[1], rhs[1], out[1]);
  ivyAddV4ToV4(lhs[2], rhs[2], out[2]);
  ivyAddV4ToV4(lhs[3], rhs[3], out[3]);
}

void ivyMulM2ToM2(IvyM2 lhs, IvyM2 rhs, IvyM2 out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyMulM3ToM3(IvyM3 lhs, IvyM3 rhs, IvyM3 out) {
  IVY_UNUSED(lhs);
  IVY_UNUSED(rhs);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyMulM4ToM4(IvyM4 lhs, IvyM4 rhs, IvyM4 out) {
  /* clang-format off */
  float a00 = lhs[0][0], a01 = lhs[0][1], a02 = lhs[0][2], a03 = lhs[0][3], // NOLINT
        a10 = lhs[1][0], a11 = lhs[1][1], a12 = lhs[1][2], a13 = lhs[1][3],
        a20 = lhs[2][0], a21 = lhs[2][1], a22 = lhs[2][2], a23 = lhs[2][3],
        a30 = lhs[3][0], a31 = lhs[3][1], a32 = lhs[3][2], a33 = lhs[3][3],

        b00 = rhs[0][0], b01 = rhs[0][1], b02 = rhs[0][2], b03 = rhs[0][3],
        b10 = rhs[1][0], b11 = rhs[1][1], b12 = rhs[1][2], b13 = rhs[1][3],
        b20 = rhs[2][0], b21 = rhs[2][1], b22 = rhs[2][2], b23 = rhs[2][3],
        b30 = rhs[3][0], b31 = rhs[3][1], b32 = rhs[3][2], b33 = rhs[3][3];
  /* clang-format on */

  out[0][0] = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
  out[0][1] = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
  out[0][2] = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
  out[0][3] = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;
  out[1][0] = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
  out[1][1] = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
  out[1][2] = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
  out[1][3] = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;
  out[2][0] = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
  out[2][1] = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
  out[2][2] = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
  out[2][3] = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;
  out[3][0] = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
  out[3][1] = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
  out[3][2] = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
  out[3][3] = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;
}

void ivyMakeRotateM4(float angle, IvyV3 axis, IvyM4 out) {
  IVY_UNUSED(angle);
  IVY_UNUSED(axis);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyTranslateM4(IvyV3 offset, IvyM4 inOut) {
  IVY_UNUSED(offset);
  IVY_UNUSED(inOut);
}

void ivyCreateOrthographicM4(float left, float right, float bottom, float top,
    float near, float far, IvyM4 out) {
  IVY_UNUSED(left);
  IVY_UNUSED(right);
  IVY_UNUSED(bottom);
  IVY_UNUSED(top);
  IVY_UNUSED(near);
  IVY_UNUSED(far);
  IVY_UNUSED(out);
  IVY_TODO();
}

static float ivyTan(float value) { return tanf(value); }

void ivyCreatePerspectiveM4(float fov, float ratio, float near, float far,
    IvyM4 out) {
  float const focalLength = 1.0F / ivyTan(fov * 0.5F);
  float const inverseOfFarNear = 1.0F / (far - near);

  out[0][0] = focalLength / ratio;
  out[0][1] = 0.0F;
  out[0][2] = 0.0F;
  out[0][3] = 0.0F;

  out[1][0] = 0.0F;
  out[1][1] = focalLength;
  out[1][2] = 0.0F;
  out[1][3] = 0.0F;

  out[2][0] = 0.0F;
  out[2][1] = 0.0F;
  out[2][2] = -far * inverseOfFarNear;
  out[2][3] = -1.0F;

  out[3][0] = 0.0F;
  out[3][1] = 0.0F;
  out[3][2] = -far * near * inverseOfFarNear;
  out[3][3] = 0.0F;
}

void ivyCreateLookM4(IvyV3 eye, IvyV3 direction, IvyV3 up, IvyM4 out) {
  IvyV3 at;

  ivyAddV3ToV3(eye, direction, at);
  ivyCreateLookAtM4(eye, at, up, out);
}

void ivyCreateLookAtM4(IvyV3 eye, IvyV3 at, IvyV3 up, IvyM4 out) {
  IvyV3 xAxis, yAxis, zAxis;

  ivySubV3ToV3(at, eye, zAxis);
  ivyNormalizeV3(zAxis);

  ivyCrossV3ToV3(zAxis, up, xAxis);
  ivyNormalizeV3(xAxis);

  ivyCrossV3ToV3(xAxis, zAxis, yAxis);
  ivyNormalizeV3(yAxis);

  out[0][0] = xAxis[0];
  out[0][1] = yAxis[0];
  out[0][2] = zAxis[0];
  out[0][3] = 0.0F;

  out[1][0] = xAxis[1];
  out[1][1] = yAxis[1];
  out[1][2] = zAxis[1];
  out[1][3] = 0.0F;

  out[2][0] = xAxis[2];
  out[2][1] = yAxis[2];
  out[2][2] = zAxis[2];
  out[2][3] = 0.0F;

  out[3][0] = -ivyDotV3ToV3(xAxis, eye);
  out[3][1] = -ivyDotV3ToV3(yAxis, eye);
  out[3][2] = -ivyDotV3ToV3(zAxis, eye);
  out[3][3] = 0.0F;
}

void ivyCreateDirectionV3(float pitch, float yaw, IvyV3 up, IvyV3 out) {
  IVY_UNUSED(pitch);
  IVY_UNUSED(yaw);
  IVY_UNUSED(up);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyRotateM4(IvyM4 matrix, float angle, IvyV3 axis, IvyM4 out) {
  IVY_UNUSED(matrix);
  IVY_UNUSED(angle);
  IVY_UNUSED(axis);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyGetDistanceBetweenV2AndV2(IvyV2 from, IvyV2 to) {
  IVY_UNUSED(from);
  IVY_UNUSED(to);
  IVY_TODO();
}
void ivyGetDistanceBetweenV3AndV3(IvyV3 from, IvyV3 to) {
  IVY_UNUSED(from);
  IVY_UNUSED(to);
  IVY_TODO();
}

void ivyGetDistanceBetweenV4AndV4(IvyV4 from, IvyV4 to) {
  IVY_UNUSED(from);
  IVY_UNUSED(to);
  IVY_TODO();
}

void ivyMixV3(IvyV3 from, IvyV3 to, float weight, IvyV3 out) {
  IVY_UNUSED(from);
  IVY_UNUSED(to);
  IVY_UNUSED(weight);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyMixV4(IvyV4 from, IvyV4 to, float weight, IvyV4 out) {
  IVY_UNUSED(from);
  IVY_UNUSED(to);
  IVY_UNUSED(weight);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyQ4AsM4(IvyQ4 quaternion, IvyM3 out) {
  IVY_UNUSED(quaternion);
  IVY_UNUSED(out);
  IVY_TODO();
}

void ivyScaleM4ByV3(IvyM4 matrix, IvyV3 scale, IvyM4 out) {
  IVY_UNUSED(matrix);
  IVY_UNUSED(scale);
  IVY_UNUSED(out);
  IVY_TODO();
}
