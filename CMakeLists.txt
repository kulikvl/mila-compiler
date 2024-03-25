cmake_minimum_required(VERSION 3.25.1)
project(mila-compiler VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")

add_subdirectory(src/main)
add_subdirectory(src/test)
