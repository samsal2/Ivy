
add_subdirectory(Unity)
add_subdirectory(glfw)
add_subdirectory(stb)
add_subdirectory(cgltf)
# add_subdirectory(freetype)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Unity
  glfw
  stb
  cgltf
  # freetype
)
