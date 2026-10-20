set(TOYLIB_TEST_INCLUDE_DIRS    ${CMAKE_CURRENT_SOURCE_DIR} ${TOYLIB_INCLUDE_DIRS})
set(DOCTEST_DIR                 "${PROJECT_SOURCE_DIR}/third_party/doctest")

if(NOT EXISTS "${DOCTEST_DIR}/doctest/doctest.h")
    message(FATAL_ERROR
            "找不到 doctest 子模块：${DOCTEST_DIR}\n"
            "请先执行  git submodule update --init --recursive")
endif()

include(${DOCTEST_DIR}/scripts/cmake/doctest.cmake)

# 只取doctest的头文件目录, 不执行它自带的CMakeLists从而避免注入install规则
add_library(doctest_interface INTERFACE)
target_include_directories(doctest_interface SYSTEM INTERFACE ${DOCTEST_DIR})
target_compile_definitions(doctest_interface INTERFACE DOCTEST_CONFIG_USE_STD_HEADERS)
add_library(doctest::doctest ALIAS doctest_interface)

# 添加一个测试可执行文件，并把其中的doctest用例逐个注册到CTest
#
# 用法:
#   toylib_add_test(NAME     <模块名>            # 必填
#                   LEVEL    <级别数字>          # 可选
#                   SOURCES  <源文件>...         # 必填
#                   [TIMEOUT <秒>])              # 默认 30
#
# 可执行文件名为"<模块名>_l<级别>_test", 例如NAME seqlist LEVEL 1 -> seqlist_l1_test
# 省略LEVEL时直接用 <模块名>_test
#
# CTest label来自doctest的TEST_SUITE名字(ADD_LABELS ON)
# TEST_SUITE_BEGIN("seqlist_l1")要与NAME/LEVEL拼出来的名字保持一致
function(toylib_add_test)
    cmake_parse_arguments(ARG "" "NAME;LEVEL;TIMEOUT" "SOURCES" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "toylib_add_test: 缺少 NAME")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "toylib_add_test(${ARG_NAME}): 缺少 SOURCES")
    endif()
    if(NOT ARG_TIMEOUT)
        set(ARG_TIMEOUT 30)
    endif()

    set(base "${ARG_NAME}")
    if(ARG_LEVEL)
        string(APPEND base "_l${ARG_LEVEL}")
    endif()

    set(target "${base}_test")
    add_executable("${target}" "test_main.cc" ${ARG_SOURCES})

    target_include_directories(${target} PRIVATE ${TOYLIB_TEST_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE toylib doctest::doctest)

    doctest_discover_tests(${target}
            TEST_PREFIX "${base}: "
            ADD_LABELS ON
            PROPERTIES TIMEOUT ${ARG_TIMEOUT})
endfunction()