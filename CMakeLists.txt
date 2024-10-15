cmake_minimum_required(VERSION 3.30)
project(arbor CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_subdirectory(lib)
add_subdirectory(src)