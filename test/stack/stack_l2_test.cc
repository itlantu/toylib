/*
 * L2边界级, 覆盖参数校验与容量边界, 并验证操作失败后状态不变
 */

#include <cstdint>

#include "doctest/doctest.h"
#include "stack_support.h"

using toylib_test::make_stack;
using toylib_test::Value;

TEST_SUITE_BEGIN("[Stack][L2]");

/**
 * 校验init拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("init rejects a null stack pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, stack_init(nullptr, 4));
}

/**
 * 校验init校验容量下界: 0不是合法容量
 * @note 容量下界封闭, 1是合法的最小值, 和seq_list_create的判定保持一致
 */
TEST_CASE("init rejects a zero capacity") {
    Stack stack{};

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_VALUE, stack_init(&stack, 0));
}

/**
 * 校验init校验容量上界: 超过MAX_CAPACITY都要拒绝
 * @note 上界是闭区间, MAX_CAPACITY本身合法, 加一就不行
 */
TEST_CASE("init rejects a capacity above the max") {
    Stack stack{};

    SUBCASE("capacity is max plus one") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_VALUE, stack_init(&stack, TOYLIB_STACK_MAX_CAPACITY + 1));
    }
    SUBCASE("capacity is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_VALUE, stack_init(&stack, SIZE_MAX));
    }
}

/**
 * 校验push拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("push rejects a null stack pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, stack_push(nullptr, 1));
}

/**
 * 校验满栈时push返回CAPACITY_FULL, 且不破坏已有内容
 * @note 容量取1, 让"满"和"空"只差一个元素, 更容易看出长度有没有被改动
 */
TEST_CASE("push on a full stack reports capacity full") {
    Stack stack = make_stack(1, {7});

    CHECK_EQ(TOYLIB_ERR_CAPACITY_FULL, stack_push(&stack, 8));

    CHECK_EQ(stack.length_, 1);
    CHECK_STACK(stack, 7);
    stack_free(&stack);
}

/**
 * 校验容量只在出栈之后才腾出来, 出栈后能继续压入
 * @note push被拒时不该动length_, 否则这个"弹一个再压一个"的流程会错位
 */
TEST_CASE("push succeeds again after a pop frees a slot") {
    Stack stack = make_stack(1, {7});

    CHECK_EQ(TOYLIB_ERR_CAPACITY_FULL, stack_push(&stack, 8));

    Value value = 0;
    REQUIRE_EQ(TOYLIB_OK, stack_pop(&stack, &value));
    CHECK_EQ(value, 7);

    CHECK_EQ(TOYLIB_OK, stack_push(&stack, 9));
    CHECK_STACK(stack, 9);
    stack_free(&stack);
}

/**
 * 校验空栈pop返回EMPTY, 出参和长度都保持原样
 */
TEST_CASE("pop on an empty stack reports empty") {
    Stack stack = make_stack(4, {});

    Value value = 42;
    CHECK_EQ(TOYLIB_ERR_EMPTY, stack_pop(&stack, &value));

    CHECK_EQ(value, 42);
    CHECK_EQ(stack.length_, 0);
    stack_free(&stack);
}

/**
 * 校验空栈top返回EMPTY, 出参保持原样
 * @note 读不到栈顶时不能往调用方的变量里写东西
 */
TEST_CASE("top on an empty stack reports empty") {
    Stack stack = make_stack(4, {});

    Value value = 42;
    CHECK_EQ(TOYLIB_ERR_EMPTY, stack_top(&stack, &value));

    CHECK_EQ(value, 42);
    CHECK_EQ(stack.length_, 0);
    stack_free(&stack);
}

/**
 * 校验top拒绝空出参, 返回参数空指针错误
 * @note 出参是唯一的返回值载体, 为空就没地方写结果
 */
TEST_CASE("top rejects a null out parameter") {
    Stack stack = make_stack(4, {1});

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, stack_top(&stack, nullptr));

    stack_free(&stack);
}

/**
 * 校验pop允许空出参, 此时只弹出不取值
 * @note 和seq_list_delete/dlist_delete的出参约定一致: 为空指针则只做动作
 */
TEST_CASE("pop accepts a null out parameter") {
    Stack stack = make_stack(4, {1, 2});

    CHECK_EQ(TOYLIB_OK, stack_pop(&stack, nullptr));

    CHECK_EQ(stack.length_, 1);
    CHECK_STACK(stack, 1);
    stack_free(&stack);
}

/**
 * 校验未初始化的栈上四个操作都返回NOT_INITIALIZED
 * @note 零初始化的栈data_为空, 应该被识别出来, 而不是让空指针参与下标运算;
 *       pop这里必须传有效出参, 否则会先撞上"出参为空"的检查
 */
TEST_CASE("operations on an uninitialized stack report not initialized") {
    Stack stack{};

    SUBCASE("push") {
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, stack_push(&stack, 1));
    }
    SUBCASE("pop") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, stack_pop(&stack, &value));
    }
    SUBCASE("top") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, stack_top(&stack, &value));
    }
    SUBCASE("free") {
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, stack_free(&stack));
    }
}

/**
 * 校验length和free的参数校验
 * @note 出参是唯一的返回值载体, 为空就没地方写结果
 */
TEST_CASE("length and free reject invalid arguments") {
    SUBCASE("length rejects a null stack pointer") {
        size_t length = 0;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, stack_length(nullptr, &length));
    }
    SUBCASE("length rejects a null out parameter") {
        Stack stack = make_stack(4, {});
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, stack_length(&stack, nullptr));
        stack_free(&stack);
    }
    SUBCASE("free rejects a null stack pointer") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, stack_free(nullptr));
    }
}

/**
 * 校验重复free被未初始化检查拦下
 * @note 第一次free必须把data_置空, 否则第二次会去free一个悬空指针
 */
TEST_CASE("free rejects a stack that was already freed") {
    Stack stack = make_stack(4, {1, 2, 3});

    CHECK_EQ(TOYLIB_OK, stack_free(&stack));
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, stack_free(&stack));
}

TEST_SUITE_END();
