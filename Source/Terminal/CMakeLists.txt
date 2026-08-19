find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

add_library(Terminal
  Terminal.cpp
  TerminalApp.cpp
)
target_include_directories(Terminal PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(Terminal PUBLIC
  MineOS
  PkgConfig::SDL2
)
