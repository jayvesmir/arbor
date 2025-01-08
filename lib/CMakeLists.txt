cmake_minimum_required(VERSION 3.30)

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
add_subdirectory(stb-cmake)

find_package(Vulkan REQUIRED)

add_library(
    imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp

    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC imgui/ imgui/backends)
target_link_libraries(imgui PRIVATE SDL3-static Vulkan::Vulkan)

add_library(${PROJECT_NAME}_external INTERFACE)

target_link_libraries(${PROJECT_NAME}_external INTERFACE fmt spdlog SDL3-static glm shaderc_combined imgui stb Vulkan::Vulkan)