enable_testing()

add_executable(IvyTestVectorMath IvyTestVectorMath.c)

target_link_libraries(IvyTestVectorMath ${PROJECT_NAME} Unity)

add_test(IvyTestVectorMathTest IvyTestVectorMath)

