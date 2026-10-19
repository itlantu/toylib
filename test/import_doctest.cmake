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
function(toylib_add_test target_suffix)
    set(target "test_${target_suffix}")
    add_executable("${target}" "test_main.cc" ${ARGN})

    target_include_directories(${target} PRIVATE ${TOYLIB_TEST_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE toylib doctest::doctest)

    doctest_discover_tests(${target} TEST_PREFIX "[${target_suffix}] " PROPERTIES TIMEOUT 30)
endfunction()