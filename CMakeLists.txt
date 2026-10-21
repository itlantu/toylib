cmake_minimum_required(VERSION 3.25)
project(toylib LANGUAGES C CXX)

# 构建时用clang-format检查代码格式, 只检查不修改文件
option(TOYLIB_FORMAT_CHECK "构建时用clang-format检查代码格式" OFF)

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

if(TOYLIB_FORMAT_CHECK)
    find_program(TOYLIB_CLANG_FORMAT_EXECUTABLE
            NAMES clang-format clang-format.exe
            DOC "clang-format可执行文件, 从PATH中查找")

    if(NOT TOYLIB_CLANG_FORMAT_EXECUTABLE)
        message(FATAL_ERROR
                "TOYLIB_FORMAT_CHECK已打开, 但找不到clang-format程序")
    endif()

    # CONFIGURE_DEPENDS: 新增源文件后, 下次构建会自动重新配置并更新列表
    file(GLOB_RECURSE TOYLIB_FORMAT_FILES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/test/*.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cc"
            "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h")

    # ALL: 每次构建都检查; --dry-run只报告差异, --Werror让差异变成构建失败
    add_custom_target(format-check ALL
            COMMAND "${TOYLIB_CLANG_FORMAT_EXECUTABLE}" --version
            COMMAND "${TOYLIB_CLANG_FORMAT_EXECUTABLE}"
                    --dry-run --Werror --style=file --fallback-style=none
                    ${TOYLIB_FORMAT_FILES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "clang-format检查代码格式"
            VERBATIM)
endif()