cmake_minimum_required(VERSION 3.23.1)

project(cimgui)

add_library(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        CXX_STANDARD          11 
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF)

target_include_directories(${PROJECT_NAME} PUBLIC 
	${PROJECT_NAME})

if (EXISTS ${PROJECT_NAME}/imgui/imgui_tables.cpp)
	set(TABLES_SOURCE "imgui/imgui_tables.cpp")
else()
	set(TABLES_SOURCE "")
endif()

target_sources(${PROJECT_NAME} PRIVATE
  ${PROJECT_NAME}/cimgui.cpp
  ${PROJECT_NAME}/imgui/imgui.cpp
  ${PROJECT_NAME}/imgui/imgui_draw.cpp
  ${PROJECT_NAME}/imgui/imgui_demo.cpp
  ${PROJECT_NAME}/imgui/imgui_widgets.cpp
	# ${PROJECT_NAME}/imgui/misc/freetype/imgui_freetype.cpp
	${TABLE_SOURCE})

target_include_directories(${PROJECT_NAME} 
	PUBLIC
		${PROJECT_NAME}/
		${PROJECT_NAME}
		${PROJECT_NAME}/imgui
		${PROJECT_NAME}/generator/output
)

target_compile_definitions(${PROJECT_NAME} PUBLIC 
	# FIXME(samuel): add freetype support
	# CIMGUI_FREETYPE=1
	IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1)

# target_link_libraries(${PROJECT_NAME} PRIVATE freetype)

target_compile_options(${PROJECT_NAME} PUBLIC
	"$<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:"
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
    -Wno-comment # C99 extension
    -Wno-variadic-macros # C99 extennsions
    -Wno-typedef-redefinition # C11 extensions 
	">"
	"$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>,$<CONFIG:Debug>>:"
		-O0
		-g
		-fsanitize=address
		-fsanitize=undefined
		-fsanitize=bounds
	">"
	"$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>,$<CONFIG:Release>>:"
		-Ofast
	">")

# link options
target_link_options(${PROJECT_NAME} PUBLIC
	"$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>,$<CONFIG:Debug>>:"
		-fsanitize=address
		-fsanitize=undefined
		-fsanitize=bounds
	">")
