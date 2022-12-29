cmake_minimum_required(VERSION 3.23.1)

project(Ivy)

# find external libraries
find_package(Vulkan REQUIRED)

# declare the library 
add_library(${PROJECT_NAME})

# features
set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        C_STANDARD          90
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS        OFF)

# compile options
target_compile_options(${PROJECT_NAME} PUBLIC
	"$<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:"
		-Wall
		-Wshadow
		-Wextra
		-Werror
		-Wpedantic
		-pedantic-errors
		-Walloca 
		-Wvla 
		-fdiagnostics-color=always

    -Wno-long-long # C99 extension
    -Wno-comment # C99 extension
    -Wno-typedef-redefinition # C11 extensions (vulkan requires it)
	">"
	"$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Debug>>:"
		-O0
		-g
		-fsanitize=address
		-fsanitize=undefined
		-fsanitize=bounds
	">"
	"$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Release>>:"
		-Ofast
	">")

# link options
target_link_options(${PROJECT_NAME} PUBLIC
	"$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Debug>>:"
		-fsanitize=address
		-fsanitize=undefined
		-fsanitize=bounds
	">")

# link libraries
target_link_libraries(${PROJECT_NAME} 
  PUBLIC ${Vulkan_LIBRARIES})

# includes
target_include_directories(${PROJECT_NAME} 
  PUBLIC ${Vulkan_INCLUDE_DIR} Source)

# add sources
add_subdirectory(Dependencies)
add_subdirectory(Source)
add_subdirectory(GLSL)
add_subdirectory(Tests)

add_executable("${PROJECT_NAME}Main" Source/main.c)
target_link_libraries("${PROJECT_NAME}Main" PRIVATE ${PROJECT_NAME})


