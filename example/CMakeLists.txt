cmake_minimum_required(VERSION 3.30)

file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME}_example)
target_sources(${PROJECT_NAME}_example
    PUBLIC
        ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME}_external ${PROJECT_NAME})