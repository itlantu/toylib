/*
 * 测试框架层的公共辅助
 *
 * 放跨模块复用的东西: 错误码的打印, L3随机序列共用的下标生成器, 参考模型转换
 */

#ifndef TOYLIB_TEST_TEST_SUPPORT_H
#define TOYLIB_TEST_TEST_SUPPORT_H

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ostream>
#include <random>
#include <vector>

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

namespace toylib_test {

/**
 * 生成一个偏向边界的下标
 * @param rng 随机数引擎
 * @param length 当前长度
 * @return 下标, 大部分落在合法区间, 少数越界
 * @note 均匀随机会很少碰到0/length/SIZE_MAX这些关键点, 这里刻意加权;
 *       seqlist和dlist的随机序列测试共用这一份实现, 种子固定时取数顺序也就固定了
 */
inline size_t pick_boundary_index(std::mt19937& rng, const size_t length) {
    switch (rng() % 6) {
        case 0:
            return 0;
        case 1:
            return length == 0 ? 0 : length - 1;
        case 2:
            return length;
        case 3:
            return length + 1;
        case 4:
            return SIZE_MAX;
        default:
            return length == 0 ? 0 : rng() % (length + 1);
    }
}

/**
 * 把参考模型转成std::vector, 用来和模块自己的to_values结果直接比较
 * @param model 参考模型, 需要有正向迭代器
 * @return 元素拷贝, 顺序和模型一致
 * @note 必须写全构造参数, 用花括号初始化的话, 元素类型一旦能接住迭代器
 *       (例如VALUE_TYPE被改成指针)就会优先匹配initializer_list, 悄悄语义都变了
 */
template <typename Model>
inline std::vector<typename Model::value_type> to_model(const Model& model) {
    return std::vector<typename Model::value_type>(model.begin(), model.end());
}

/**
 * 把参考模型逆序转成std::vector, 用来核对prev_链这种反向遍历
 * @param model 参考模型, 需要有正向迭代器
 * @return 元素拷贝, 顺序和模型相反
 */
template <typename Model>
inline std::vector<typename Model::value_type> to_model_reversed(const Model& model) {
    return std::vector<typename Model::value_type>(model.rbegin(), model.rend());
}

}  // namespace toylib_test

#endif  // TOYLIB_TEST_TEST_SUPPORT_H
