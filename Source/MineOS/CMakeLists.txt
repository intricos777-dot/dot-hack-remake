find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

add_library(MineOS
  Renderer.cpp
  Widgets.cpp
  Apps.cpp
  World.cpp
  MineOS.cpp
  WebBrowserApp.cpp
  AvatarDef.cpp
  SaveSystem.cpp
  DiscBoot.cpp
  npc_db.cpp
  world_data.cpp
  combat_system.cpp
  character.cpp
)
target_include_directories(MineOS PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/Source/World3D
  ${CMAKE_SOURCE_DIR}/Source/Terminal
)
target_link_libraries(MineOS PUBLIC
  Aida
  World3D
  Terminal
  PkgConfig::SDL2
  PkgConfig::SDL2TTF
)
target_compile_definitions(MineOS PUBLIC
  MINE_OS_SCRIPTS_DIR="${CMAKE_SOURCE_DIR}/scripts"
  MINE_OS_CONTENT_DIR="${CMAKE_SOURCE_DIR}/Content"
)
