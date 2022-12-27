#ifndef IVY_VECTOR_MATH_H
#define IVY_VECTOR_MATH_H

typedef float IvyV2[2];
typedef float IvyV3[3];
typedef float IvyV4[4];

typedef IvyV2 IvyM2[2];
typedef IvyV3 IvyM3[3];
typedef IvyV4 IvyM4[4];

void ivyZeroV2(IvyV2 out);

void ivyAddV2ToV2(IvyV2 lhs, IvyV2 rhs, IvyV2 out);

#endif
