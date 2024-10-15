cmake_minimum_required(VERSION 3.30)

file(GLOB SOURCES 
    LIST_DIRECTORIES false
    ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

file(GLOB MODULE_SOURCES 
    LIST_DIRECTORIES false
    ${CMAKE_CURRENT_SOURCE_DIR} *.cppm)

file(GLOB_RECURSE ENGINE_MODULE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} "engine/*.cppm")

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PUBLIC
        ${SOURCES}
)

add_library(${PROJECT_NAME}_global_modules)
target_sources(${PROJECT_NAME}_global_modules
    PUBLIC FILE_SET CXX_MODULES FILES
        ${MODULE_SOURCES}
)

add_library(${PROJECT_NAME}_engine)
target_sources(${PROJECT_NAME}_engine
    PUBLIC FILE_SET CXX_MODULES FILES
        ${ENGINE_MODULE_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_engine PRIVATE ${PROJECT_NAME}_external)
target_link_libraries(${PROJECT_NAME}_engine PUBLIC ${PROJECT_NAME}_global_modules)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_external ${PROJECT_NAME}_engine)