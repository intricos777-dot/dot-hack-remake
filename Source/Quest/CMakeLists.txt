add_library(Quest Quest.cpp)
target_include_directories(Quest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(Quest PUBLIC MINE_OS_CONTENT_DIR="${CMAKE_SOURCE_DIR}/Content")
