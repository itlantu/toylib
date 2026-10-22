/*
 * 测试框架层的公共辅助
 *
 */

#ifndef TOYLIB_TEST_TEST_SUPPORT_H
#define TOYLIB_TEST_TEST_SUPPORT_H

#include <ostream>

#include "doctest/doctest.h"
#include "toylib/utils/error.h"

/**
 * 让doctest在断言失败时打印错误码的枚举名, 而不是裸数字
 * @param stream 目标输出流
 * @param error 被测函数返回的错误码
 * @return 目标输出流
 * @note doctest对"没有operator<<的枚举"会强制转成底层类型打印
 *       (should_stringify_as_underlying_type), 所以只写toString或StringMaker都不管用,
 *       必须先提供这个运算符
 */
inline std::ostream& operator<<(std::ostream& stream, const ToylibError error) {
    return stream << toylib_error_name(error);
}

#endif  // TOYLIB_TEST_TEST_SUPPORT_H
