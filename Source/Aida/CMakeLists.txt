find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

add_library(Aida
  GameState.cpp
  Bridge.cpp
  LiaBridge.cpp
)
target_include_directories(Aida PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
