cmake_minimum_required(VERSION 3.16)
project(tetris C)

set(CMAKE_C_STANDARD 11)

file(GLOB SRC_FILES "src/*.c")

add_executable(tetris ${SRC_FILES})

find_package(sdl2 CONFIG REQUIRED)
find_package(sdl2_ttf CONFIG REQUIRED)

Include_directories(tetris ${SDL2_INCLUDE_DIRS})

target_link_libraries(tetris PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_ttf::SDL2_ttf)
