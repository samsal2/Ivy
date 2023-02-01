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

  ivyZeroV2(&out);

  TEST_ASSERT_EQUAL_FLOAT(0.0F, out.x);
  TEST_ASSERT_EQUAL_FLOAT(0.0F, out.y);

}

int main(void) {
  UNITY_BEGIN();
 
  RUN_TEST(testZeroV2);
 
  return UNITY_END();
}
