/*
 * L1契约级, 锁定错误码到字符串名字的映射
 */

#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "test_support.h"
#include "toylib/utils/error.h"

TEST_SUITE_BEGIN("[Error][L1]");

/**
 * 校验每个错误码都映射到自己的枚举名
 * @note 表驱动写法: 内部靠下标查表, 枚举插入新值时下标整体错位, 这里会逐个报出来
 */
TEST_CASE("each error code maps to its own name") {
    struct Entry {
        ToylibError error;
        const char* name;
    };

    const std::vector<Entry> table = {
      {TOYLIB_OK, "TOYLIB_OK"},
      {TOYLIB_ERR_BAD_ARG_NULL, "TOYLIB_ERR_BAD_ARG_NULL"},
      {TOYLIB_ERR_BAD_ARG_VALUE, "TOYLIB_ERR_BAD_ARG_VALUE"},
      {TOYLIB_ERR_NOT_INITIALIZED, "TOYLIB_ERR_NOT_INITIALIZED"},
      {TOYLIB_ERR_OUT_OF_RANGE, "TOYLIB_ERR_OUT_OF_RANGE"},
      {TOYLIB_ERR_CAPACITY_FULL, "TOYLIB_ERR_CAPACITY_FULL"},
      {TOYLIB_ERR_EMPTY, "TOYLIB_ERR_EMPTY"},
      {TOYLIB_ERR_NOMEM, "TOYLIB_ERR_NOMEM"},
      {TOYLIB_ERR_UNKNOWN, "TOYLIB_ERR_UNKNOWN"},
    };

    for (const Entry& entry : table) {
        // 用std::string比较, 直接比const char*比的是指针地址
        CHECK_EQ(std::string(toylib_error_name(entry.error)), std::string(entry.name));
    }
}

/**
 * 校验名字之间互不重复
 * @note 错位时如果两个下标恰好指向同一个名字, 单看某一条断言发现不了
 */
TEST_CASE("error names are distinct") {
    const std::vector<ToylibError> codes = {
      TOYLIB_OK,
      TOYLIB_ERR_BAD_ARG_NULL,
      TOYLIB_ERR_BAD_ARG_VALUE,
      TOYLIB_ERR_NOT_INITIALIZED,
      TOYLIB_ERR_OUT_OF_RANGE,
      TOYLIB_ERR_CAPACITY_FULL,
      TOYLIB_ERR_EMPTY,
      TOYLIB_ERR_NOMEM,
      TOYLIB_ERR_UNKNOWN,
    };

    for (size_t i = 0; i < codes.size(); ++i) {
        for (size_t j = i + 1; j < codes.size(); ++j) {
            CHECK_NE(std::string(toylib_error_name(codes[i])), std::string(toylib_error_name(codes[j])));
        }
    }
}

TEST_SUITE_END();
