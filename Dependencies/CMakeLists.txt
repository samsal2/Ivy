
add_subdirectory(Unity)
add_subdirectory(glfw)
add_subdirectory(stb)
add_subdirectory(cgltf)
add_subdirectory(FreeType)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Unity
  glfw
  stb
  cgltf
  FreeType
)
