#include "IvyVectorMath.h"

void ivyZeroV2(IvyV2 out)
{
  out[0] = 0.0F;
  out[1] = 0.0F;
}

void ivyAddV2ToV2(IvyV2 lhs, IvyV2 rhs, IvyV2 out)
{
  out[0] = lhs[0] + rhs[0];
  out[1] = lhs[1] + rhs[1];
}
