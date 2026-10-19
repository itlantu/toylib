cmake_minimum_required(VERSION 3.25)
project(toylib LANGUAGES C CXX)

set(CMAKE_C_STANDARD        23)
set(CMAKE_C_EXTENSIONS      OFF)

set(CMAKE_CXX_STANDARD      20)
set(CMAKE_CXX_EXTENSIONS    OFF)

if(MSVC)
    add_compile_options("/utf-8" "/Zc:__cplusplus")
endif()

set(TOYLIB_INCLUDE_DIRS             "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(GLOB_RECURSE TOYLIB_SOURCES    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(toylib STATIC ${TOYLIB_SOURCES})
target_include_directories(toylib PRIVATE ${TOYLIB_INCLUDE_DIRS})

enable_testing()
add_subdirectory("test")