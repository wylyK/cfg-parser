cmake_minimum_required(VERSION 3.22)
project(parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(example)