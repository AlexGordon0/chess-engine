cmake_minimum_required(VERSION 3.28)

project(gorbot)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED CONFIG)
find_package(SDL2_image REQUIRED CONFIG)

add_executable(gorbot 
  src/main.cpp
  src/board.cpp
  src/move.cpp
)

target_link_libraries(gorbot
  SDL2::SDL2
  SDL2_image::SDL2_image
)
