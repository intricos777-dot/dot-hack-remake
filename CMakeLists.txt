cmake_minimum_required(VERSION 3.20)
project(dot-hack-remake LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

add_subdirectory(Source/Common)
add_subdirectory(Source/Rendering)
add_subdirectory(Source/Input)
add_subdirectory(Source/Audio)
add_subdirectory(Source/Physics)
add_subdirectory(Source/Networking)
add_subdirectory(Source/WorldState)
add_subdirectory(Source/Timeline)
add_subdirectory(Source/CharacterCreator)
add_subdirectory(Source/Customization)
add_subdirectory(Source/Quest)
add_subdirectory(Source/ModSDK)
add_subdirectory(Source/UI)
add_subdirectory(Source/VR)
add_subdirectory(Source/Aida)
add_subdirectory(Source/Terminal)
add_subdirectory(Source/MineOS)
add_subdirectory(Source/World3D)

# Add the main executable - using Source/main.cpp (framework initialization)
file(GLOB MAINSRC "Source/main.cpp")
add_executable(dot-hack-remake ${MAINSRC})

target_include_directories(dot-hack-remake PUBLIC
  Source/Common
  Source/Rendering
  Source/Input
  Source/Audio
  Source/Physics
  Source/Networking
  Source/WorldState
  Source/Timeline
  Source/CharacterCreator
  Source/Customization
  Source/Quest
  Source/ModSDK
  Source/UI
  Source/VR
  Source/Aida
  Source/Terminal
  Source/MineOS
  Source/World3D
  /usr/include/SDL2
  /usr/include/freetype2
)

target_link_libraries(dot-hack-remake PUBLIC
  Common
  Rendering
  Input
  Audio
  Physics
  WorldState
  Timeline
  CharacterCreator
  Customization
  Quest
  ModSDK
  UI
  VR
  Aida
  Terminal
  MineOS
  World3D
  ${SDL2_LIBRARIES}
  ${SDL2TTF_LIBRARIES})

target_link_directories(dot-hack-remake PUBLIC /home/sin/.local/lib)