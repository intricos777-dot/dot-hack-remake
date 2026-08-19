add_library(World3D STATIC
  World3D.cpp
  MeshBuilder.cpp
  CharacterMeshes.cpp
)

target_link_libraries(World3D PRIVATE MineOS ${SDL2_LIBRARIES})
target_include_directories(World3D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
