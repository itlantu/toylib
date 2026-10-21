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

# 建出可执行文件, 并把其中的doctest用例逐条注册到CTest
function(_toylib_add_test_target target prefix timeout)
    add_executable("${target}" "test_main.cc" ${ARGN})

    target_include_directories(${target} PRIVATE ${TOYLIB_TEST_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE toylib doctest::doctest)

    doctest_discover_tests(${target}
            TEST_PREFIX "${prefix}: "
            ADD_LABELS ON
            PROPERTIES TIMEOUT ${timeout})
endfunction()

# 一个测试目录 = 一个可执行文件
#
# 用法:
#   toylib_add_test_dir(NAME      <模块名>       # 必填, 可执行文件名为 <模块名>_test
#                       DIRECTORY <相对目录>     # 必填, 相对 test/ 目录
#                       [TIMEOUT  <秒>])         # 单条用例超时, 默认 60
#
# 目录下所有"*_test.cc"文件都会编进同一个可执行文件
# 用例仍然逐条注册到CTest, 每条用例由CTest单独起进程运行(--test-case=...)
function(toylib_add_test_dir)
    cmake_parse_arguments(ARG "" "NAME;DIRECTORY;TIMEOUT" "" ${ARGN})
    # 参数校验
    if(NOT ARG_NAME)
        message(FATAL_ERROR "toylib_add_test_dir: 缺少 NAME")
    endif()
    if(NOT ARG_DIRECTORY)
        message(FATAL_ERROR "toylib_add_test_dir(${ARG_NAME}): 缺少 DIRECTORY")
    endif()
    if(NOT ARG_TIMEOUT)
        set(ARG_TIMEOUT 60)
    endif()
    # 校验测试目录
    set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_DIRECTORY}")
    if(NOT IS_DIRECTORY "${test_dir}")
        message(FATAL_ERROR "toylib_add_test_dir(${ARG_NAME}): 目录不存在 ${test_dir}")
    endif()
    # 排序测试文件
    file(GLOB test_sources CONFIGURE_DEPENDS "${test_dir}/*_test.cc")
    list(SORT test_sources)
    if(NOT test_sources)
        message(FATAL_ERROR
                "toylib_add_test_dir(${ARG_NAME}): ${ARG_DIRECTORY} 下没有 *_test.cc")
    endif()
    # 构建可执行文件
    _toylib_add_test_target("${ARG_NAME}_test" "${ARG_NAME}" "${ARG_TIMEOUT}" ${test_sources})
endfunction()
