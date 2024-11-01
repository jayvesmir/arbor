cmake_minimum_required(VERSION 3.30)

file(GLOB SOURCES 
    LIST_DIRECTORIES false
    ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR} "engine/*.cpp")

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PUBLIC
        ${SOURCES}
)

add_library(${PROJECT_NAME}_engine)
target_sources(${PROJECT_NAME}_engine
    PUBLIC
        ${ENGINE_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_engine PRIVATE ${PROJECT_NAME}_external)
target_include_directories(${PROJECT_NAME}_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_external ${PROJECT_NAME}_engine)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})