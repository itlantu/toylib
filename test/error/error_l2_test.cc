/*
 * L2边界级, 覆盖枚举范围之外的取值
 */

#include <string>

#include "doctest/doctest.h"
#include "test_support.h"
#include "toylib/utils/error.h"

TEST_SUITE_BEGIN("[Error][L2]");

/**
 * 校验大于等于UNKNOWN的取值统一返回UNKNOWN
 * @note 查表之前必须先拦上界, 否则就是数组越界读
 */
TEST_CASE("a code past the last value maps to unknown") {
    CHECK_EQ(std::string(toylib_error_name(static_cast<ToylibError>(TOYLIB_ERR_UNKNOWN + 1))),
             std::string("TOYLIB_ERR_UNKNOWN"));
    CHECK_EQ(std::string(toylib_error_name(static_cast<ToylibError>(100))), std::string("TOYLIB_ERR_UNKNOWN"));
}

/**
 * 校验负数取值也返回UNKNOWN
 * @note 枚举的底层类型是有符号的, 负数会绕过"大于等于UNKNOWN"这个判断, 直接拿去当下标
 */
TEST_CASE("a negative code maps to unknown") {
    CHECK_EQ(std::string(toylib_error_name(static_cast<ToylibError>(-1))), std::string("TOYLIB_ERR_UNKNOWN"));
}

TEST_SUITE_END();
