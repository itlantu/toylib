/*
 * L1契约级, 锁定错误码到字符串名字的映射
 */

#include <cstddef>
#include <iterator>
#include <string>

#include "doctest/doctest.h"
#include "test_support.h"
#include "toylib/utils/error.h"

namespace {

struct ErrorName {
    ToylibError error_;
    const char* name_;
};

/**
 * 错误码到枚举名的完整映射, 本文件的两个用例共用
 * @note 新增枚举值时只改这一处; 漏改或错位会被下面两条用例逐条报出来
 */
constexpr ErrorName kErrorNames[] = {
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

// 两个用例都只认这张表, 所以表必须覆盖到UNKNOWN为止的每个取值,
// 新增枚举值却忘了加表就在编译期报出来, 不留静默漏测的口子
static_assert(std::size(kErrorNames) == static_cast<size_t>(TOYLIB_ERR_UNKNOWN) + 1, "kErrorNames漏了错误码");

/**
 * 取错误码的枚举名并包成std::string
 * @param error 错误码
 * @return 枚举名
 * @note 返回值必须是std::string, 直接拿const char*比较的是指针地址
 */
std::string name_of(const ToylibError error) {
    return toylib_error_name(error);
}

}  // namespace

TEST_SUITE_BEGIN("[Error][L1]");

/**
 * 校验每个错误码都映射到自己的枚举名
 * @note 表驱动写法: 内部靠下标查表, 枚举插入新值时下标整体错位, 这里会逐个报出来
 */
TEST_CASE("each error code maps to its own name") {
    for (const ErrorName& entry : kErrorNames)
        CHECK_EQ(name_of(entry.error_), entry.name_);
}

/**
 * 校验名字之间互不重复
 * @note 错位时如果两个下标恰好指向同一个名字, 单看某一条断言发现不了
 */
TEST_CASE("error names are distinct") {
    for (size_t i = 0; i < std::size(kErrorNames); ++i) {
        for (size_t j = i + 1; j < std::size(kErrorNames); ++j) {
            CHECK_NE(name_of(kErrorNames[i].error_), name_of(kErrorNames[j].error_));
        }
    }
}

TEST_SUITE_END();
