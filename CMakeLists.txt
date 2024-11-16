cmake_minimum_required(VERSION 3.30)
project(arbor CXX)

set(BUILD_SHARED_LIBS false)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(example)