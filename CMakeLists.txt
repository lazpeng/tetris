cmake_minimum_required(VERSION 3.16)
project(tetris C)

set(CMAKE_C_STANDARD 11)

file(GLOB SRC_FILES "src/*.c")

add_executable(tetris ${SRC_FILES})

find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-ttf CONFIG REQUIRED)
target_link_libraries(tetris PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_ttf)