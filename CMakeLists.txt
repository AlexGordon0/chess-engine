cmake_minimum_required(VERSION 3.28)

project(chess-engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED CONFIG)
find_package(SDL2_image REQUIRED CONFIG)

add_executable(${PROJECT_NAME} 
  src/main.cpp
  src/board.cpp
  src/move.cpp
  src/masks.cpp
  src/magics.cpp
)

add_executable(generate_magics
    src/generate_magics.cpp
    src/magics.cpp
)

target_link_libraries(${PROJECT_NAME}
  SDL2::SDL2
  SDL2_image::SDL2_image
)
