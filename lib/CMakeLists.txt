cmake_minimum_required(VERSION 3.30)

set(BUILD_SHARED_LIBS false)

option(SPDLOG_FMT_EXTERNAL "" true)

option(SDL_SHARED "" false)
option(SDL_STATIC "" true)

option(SHADERC_SKIP_TESTS "" true)
option(SHADERC_SKIP_INSTALL "" true)
option(SHADERC_SKIP_EXAMPLES "" true)

option(GLM_ENABLE_CXX_20 "" true)

add_subdirectory(fmt)
add_subdirectory(sdl)
add_subdirectory(glm)
add_subdirectory(spdlog)
add_subdirectory(shaderc)

find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME}_external INTERFACE)

target_link_libraries(${PROJECT_NAME}_external INTERFACE spdlog SDL3-static glm shaderc_combined ${Vulkan_LIBRARIES})
target_link_directories(${PROJECT_NAME}_external INTERFACE ${Vulkan_LIBRARY_DIRS})
target_include_directories(${PROJECT_NAME}_external INTERFACE ${Vulkan_INCLUDE_DIRS})