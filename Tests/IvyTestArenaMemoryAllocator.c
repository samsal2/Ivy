
#include <IvyArenaMemoryAllocator.h>
#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void testRequestTooMuchMemory(void) {
  void *data;
  IvyCode ivyCode;
  IvyArenaMemoryAllocator allocator;

  ivyCode = ivyCreateArenaMemoryAllocator(1024, &allocator);
  TEST_ASSERT_EQUAL_INT(ivyCode, IVY_OK);

  data = ivyAllocateMemory(&allocator, 1025);
  TEST_ASSERT_NULL(data);

  ivyClearMemoryAllocator(&allocator);
  ivyDestroyMemoryAllocator(&allocator);
}

void testRequestSingleAllocation(void) {
  void *data;
  IvyCode ivyCode;
  IvyArenaMemoryAllocator allocator;

  ivyCode = ivyCreateArenaMemoryAllocator(1024, &allocator);
  TEST_ASSERT_EQUAL_INT(ivyCode, IVY_OK);

  data = ivyAllocateMemory(&allocator, 1024);
  TEST_ASSERT_NOT_NULL(data);

  ivyClearMemoryAllocator(&allocator);
  ivyDestroyMemoryAllocator(&allocator);
}

void testRequestTooMuchMemoryDoubleAllocation(void) {
  void *data;
  IvyCode ivyCode;
  IvyArenaMemoryAllocator allocator;

  ivyCode = ivyCreateArenaMemoryAllocator(1024, &allocator);
  TEST_ASSERT_EQUAL_INT(ivyCode, IVY_OK);

  data = ivyAllocateMemory(&allocator, 1024);
  TEST_ASSERT_NOT_NULL(data);

  data = ivyAllocateMemory(&allocator, 1024);
  TEST_ASSERT_NULL(data);

  ivyClearMemoryAllocator(&allocator);
  ivyDestroyMemoryAllocator(&allocator);
}

void testMultipleFrees(void) {
  void *data1;
  void *data2;
  IvyCode ivyCode;
  IvyArenaMemoryAllocator allocator;

  ivyCode = ivyCreateArenaMemoryAllocator(1024, &allocator);
  TEST_ASSERT_EQUAL_INT(ivyCode, IVY_OK);

  // NOTE: whole 1024 buffer is not usable because of alignment
  data1 = ivyAllocateMemory(&allocator, 128);
  TEST_ASSERT_NOT_NULL(data1);

  data2 = ivyAllocateMemory(&allocator, 128);
  TEST_ASSERT_NOT_NULL(data2);

  ivyFreeMemory(&allocator, data2); 
  ivyFreeMemory(&allocator, data1); 

  ivyDestroyMemoryAllocator(&allocator);
}


int main(void) {
  UNITY_BEGIN();
 
  RUN_TEST(testRequestTooMuchMemory);
  RUN_TEST(testRequestSingleAllocation);
  RUN_TEST(testRequestTooMuchMemoryDoubleAllocation);
  RUN_TEST(testMultipleFrees);
 
  return UNITY_END();
}
