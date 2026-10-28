/*
 * Stack测试的辅助函数
 *
 * 只服务于test/stack目录下的用例, 不做跨模块复用
 */

#ifndef TOYLIB_TEST_STACK_SUPPORT_H
#define TOYLIB_TEST_STACK_SUPPORT_H

#include <initializer_list>
#include <vector>

#include "doctest/doctest.h"
#include "test_support.h"
#include "toylib/stack.h"

namespace toylib_test {

using Value = TOYLIB_STACK_VALUE_TYPE;

/**
 * L3随机序列测试的操作名, 下标就是用例里的op编号
 * @note 顺序必须和L3的switch保持一致; 长度顺便充当可抽取的操作数
 */
inline constexpr const char* kOpNames[] = {"push", "pop", "top", "length"};

/**
 * 从栈底到栈顶收集元素
 * @param stack 目标栈
 * @return 元素拷贝, 下标0是栈底, 末尾是栈顶
 */
inline std::vector<Value> to_values(const Stack& stack) {
    return std::vector<Value>(stack.data_, stack.data_ + stack.length_);
}

/**
 * 建一个栈并按顺序压入初值
 * @param capacity 栈容量
 * @param values 初始元素, 第一个最先压入, 也就是最后才会被弹出
 * @return 初始化好的栈, 用完需要stack_free
 * @note 内部不检查返回值, 只适合关心栈内容的用例
 */
inline Stack make_stack(const size_t capacity, const std::initializer_list<Value> values) {
    Stack stack{};
    stack_init(&stack, capacity);
    for (const Value value : values)
        stack_push(&stack, value);
    return stack;
}

}  // namespace toylib_test

// 断言栈内元素从底到顶和期望一致, 例如 CHECK_STACK(stack, 1, 2, 3)
#define CHECK_STACK(stack, ...) CHECK_EQ(toylib_test::to_values(stack), (std::vector<toylib_test::Value>{__VA_ARGS__}))

#endif  // TOYLIB_TEST_STACK_SUPPORT_H
