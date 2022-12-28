#include "IvyVectorMath.h"

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
  float magnitude = ivyGetMagnitudeOfV2(vector);
  ivyScaleV2(1 / magnitude, vector);
}

void ivyNormalizeV3(IvyV3 vector) {
  float magnitude = ivyGetMagnitudeOfV3(vector);
  ivyScaleV3(1 / magnitude, vector);
}

void ivyNormalizeV4(IvyV4 vector) {
  float magnitude = ivyGetMagnitudeOfV4(vector);
  ivyScaleV4(1 / magnitude, vector);
}

void ivyZeroV2(IvyV2 out) { ivyBroadcastV2(0.0F, out); }

void ivyZeroV3(IvyV3 out) { ivyBroadcastV3(0.0F, out); }

void ivyZeroV4(IvyV4 out) { ivyBroadcastV4(0.0F, out); }

void ivyInvM2(IvyM2 out);
void ivyInvM3(IvyM3 out);
void ivyInvM4(IvyM4 out);

void ivyCrossV2ToV2(IvyV2 lhs, IvyV2 rhs, IvyV2 out);
void ivyCrossV3ToV3(IvyV3 lhs, IvyV3 rhs, IvyV3 out);
void ivyCrossV4ToV4(IvyV4 lhs, IvyV4 rhs, IvyV4 out);

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

#if 0
void ivyAddM2ToM2(IvyM2 lhs, IvyM2 rhs, IvyM2 out);
void ivyAddM3ToM3(IvyM3 lhs, IvyM3 rhs, IvyM3 out);
void ivyAddM4ToM4(IvyM4 lhs, IvyM4 rhs, IvyM4 out);

void ivyMulM2ToM2(IvyM2 lhs, IvyM2 rhs, IvyM2 out);
void ivyMulM3ToM3(IvyM3 lhs, IvyM3 rhs, IvyM3 out);
void ivyMulM4ToM4(IvyM4 lhs, IvyM4 rhs, IvyM4 out);

void ivyMakeRotateM4(float angle, IvyV3 axis, IvyM4 out);
void ivyTranslateM4(IvyV3 offset, IvyM4 inOut);

void ivyCreateOrthographicM4(
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far,
    IvyM4 out);

void ivyCreatePerspectiveM4(
    float fov,
    float ratio,
    float near,
    float far,
    IvyM4 out);

void ivyCreateLookAtM4(IvyV3 eye, IvyV3 at, IvyV3 up, IvyM4 out);
void ivyCreateDirectionV3(float pitch, float yaw, IvyV3 up, IvyV3 out);

void ivyRotateM4(IvyM4 matrix, float angle, IvyV3 axis, IvyM4 out);

void ivyGetDistanceBetweenV2AndV2(IvyV2 from, IvyV2 to);
void ivyGetDistanceBetweenV3AndV3(IvyV3 from, IvyV3 to);
void ivyGetDistanceBetweenV4AndV4(IvyV4 from, IvyV4 to);

void ivyMixV3(IvyV3 from, IvyV3 to, float weight, IvyV3 out);
void ivyMixV4(IvyV4 from, IvyV4 to, float weight, IvyV4 out);
void ivyQ4AsM4(IvyQ4 quaternion, IvyM3 out);
void ivyScaleM4ByV3(IvyM4 matrix, IvyV3 scale, IvyM4 out);

#endif
