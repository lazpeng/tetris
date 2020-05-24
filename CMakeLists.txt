cmake_minimum_required(VERSION 3.16)
project(tetris C)

set(CMAKE_C_STANDARD 11)

file(GLOB SRC_FILES "src/*.c")

add_executable(tetris ${SRC_FILES})

target_include_directories(tetris PUBLIC "C:\\Users\\rafael.nakano\\Documents\\SDL2-2.0.12\\include")
target_link_directories(tetris PUBLIC "C:\\Users\\rafael.nakano\\Documents\\SDL2-2.0.12\\lib\\x86")
target_link_libraries(tetris SDL2main SDL2 SDL2_ttf)