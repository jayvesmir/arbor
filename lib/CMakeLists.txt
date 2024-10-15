cmake_minimum_required(VERSION 3.30)

option(SPDLOG_FMT_EXTERNAL "" true)

option(SDL_SHARED "" false)
option(SDL_STATIC "" true)

add_subdirectory(fmt)
add_subdirectory(sdl)
add_subdirectory(spdlog)

add_library(${PROJECT_NAME}_external INTERFACE)

target_link_libraries(${PROJECT_NAME}_external INTERFACE spdlog SDL3-static)