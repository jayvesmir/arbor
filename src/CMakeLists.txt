cmake_minimum_required(VERSION 3.30)

file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

set(CMAKE_CXX_STANDARD 23)


add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME}
    PUBLIC
        ${ENGINE_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_external)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)