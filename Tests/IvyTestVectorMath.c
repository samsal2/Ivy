#include <IvyVectorMath.h>
#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void testZeroV2(void) {
  IvyV2 out;

  ivyZeroV2(out);

  TEST_ASSERT_EQUAL_FLOAT(0.0F, out[0]);
  TEST_ASSERT_EQUAL_FLOAT(0.0F, out[1]);

}

void testAddV2ToV2(void) {
  IvyV2 lhs;
  IvyV2 rhs;
  IvyV2 out;

  lhs[0] = 2.0F;
  lhs[1] = 1.0F;

  rhs[0] = 1.0F;
  rhs[1] = 2.0F;

  ivyAddV2ToV2(lhs, rhs, out);

  TEST_ASSERT_EQUAL_FLOAT(3.0F, out[0]);
  TEST_ASSERT_EQUAL_FLOAT(3.0F, out[1]);
  
  ivyAddV2ToV2(out, rhs, lhs);

  TEST_ASSERT_EQUAL_FLOAT(4.0F, lhs[0]);
  TEST_ASSERT_EQUAL_FLOAT(5.0F, lhs[1]);

  ivyAddV2ToV2(lhs, rhs, rhs);

  TEST_ASSERT_EQUAL_FLOAT(5.0F, rhs[0]);
  TEST_ASSERT_EQUAL_FLOAT(7.0F, rhs[1]);

  ivyAddV2ToV2(rhs, rhs, rhs);

  TEST_ASSERT_EQUAL_FLOAT(10.0F, rhs[0]);
  TEST_ASSERT_EQUAL_FLOAT(14.0F, rhs[1]);
}

int main(void) {
  UNITY_BEGIN();
 
  RUN_TEST(testZeroV2);
  RUN_TEST(testAddV2ToV2);
 
  return UNITY_END();
}
