cmake_minimum_required(VERSION 3.15)
project(untitled C)
find_package(SDL2 REQUIRED SDL2)
set(CMAKE_C_STANDARD 99)

add_executable(untitled main.c chip8_core.c chip8_core.h graphics.c graphics.h)
target_link_libraries(untitled PRIVATE SDL2)