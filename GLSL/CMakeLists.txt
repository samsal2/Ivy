set(IVY_GLSL_SHADERS
  Basic.frag
  Basic.vert)

if ("${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}" STREQUAL "")
  message(AUTHOR_WARNING "missing Vulkan_GLSLANG_VALIDATOR_EXECUTABLE"
                         "please compile shaders manually ${GLSL_SHADERS}") 
else()
  foreach(SHADER IN LISTS IVY_GLSL_SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    set(OUT ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}.spv)
    add_custom_command(OUTPUT ${OUT}
      COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} ${SHADER} -V -o ${OUT}
      DEPENDS ${SHADER}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
      list(APPEND SPV_SHADERS ${OUT})
  endForeach()

  add_custom_target(shaders DEPENDS ${SPV_SHADERS})
  add_dependencies(${PROJECT_NAME} shaders)
endif()
