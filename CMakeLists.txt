cmake_minimum_required(VERSION 3.16)
project(tetris C)

set(CMAKE_C_STANDARD 11)

file(GLOB SRC_FILES "src/*.c" "src/*.h")

add_executable(tetris ${SRC_FILES})

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2_ttf CONFIG REQUIRED)
Include_directories(tetris ${SDL2_INCLUDE_DIRS})
target_link_libraries(tetris PRIVATE SDL2 SDL2main SDL2_TTF)
