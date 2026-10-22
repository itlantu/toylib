/*
 * L1契约级, 锁定每个API的最小可用路径和后置状态
 */

#include "doctest/doctest.h"
#include "stack_support.h"

using toylib_test::make_stack;
using toylib_test::Value;

TEST_SUITE_BEGIN("[Stack][L1]");

/**
 * 校验init按容量分配内存, 建出一个空栈
 * @note 契约: capacity_等于入参, length_为0, data_指向有效内存
 */
TEST_CASE("init allocates memory and leaves the stack empty") {
    Stack stack{};

    REQUIRE_EQ(TOYLIB_OK, stack_init(&stack, 4));

    CHECK_EQ(stack.capacity_, 4);
    CHECK_EQ(stack.length_, 0);
    CHECK(stack.data_ != nullptr);

    stack_free(&stack);
}

/**
 * 校验push把值放进栈里, 并且长度只增加一个
 * @note 容量由init决定, 压栈过程中不该变化
 */
TEST_CASE("push grows the stack by one") {
    Stack stack = make_stack(4, {});

    CHECK_EQ(TOYLIB_OK, stack_push(&stack, 10));
    CHECK_EQ(stack.length_, 1);
    CHECK_EQ(stack.capacity_, 4);

    CHECK_EQ(TOYLIB_OK, stack_push(&stack, 20));
    CHECK_EQ(stack.length_, 2);

    CHECK_STACK(stack, 10, 20);
    stack_free(&stack);
}

/**
 * 校验top返回最后压入的值
 * @note 栈顶是最后push且尚未pop的元素, 不是最早进入的那个
 */
TEST_CASE("top returns the last pushed value") {
    Stack stack = make_stack(4, {1, 2, 3});

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, stack_top(&stack, &value));

    CHECK_EQ(value, 3);
    stack_free(&stack);
}

/**
 * 校验top是只读操作, 不弹出也不改动栈
 * @note 连续读两次结果相同, 长度和内容都不该变
 */
TEST_CASE("top does not modify the stack") {
    Stack stack = make_stack(4, {1, 2, 3});

    Value first = 0;
    Value second = 0;
    CHECK_EQ(TOYLIB_OK, stack_top(&stack, &first));
    CHECK_EQ(TOYLIB_OK, stack_top(&stack, &second));

    CHECK_EQ(first, second);
    CHECK_EQ(stack.length_, 3);
    CHECK_STACK(stack, 1, 2, 3);
    stack_free(&stack);
}

/**
 * 校验pop带回原栈顶的值, 并把长度减一
 * @note 出栈后剩下的还是原来那些元素, 只是少了栈顶一个
 */
TEST_CASE("pop returns the top value and shrinks the stack") {
    Stack stack = make_stack(4, {1, 2, 3});

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, stack_pop(&stack, &value));

    CHECK_EQ(value, 3);
    CHECK_EQ(stack.length_, 2);
    CHECK_STACK(stack, 1, 2);
    stack_free(&stack);
}

/**
 * 校验pop严格按后进先出顺序吐出元素
 * @note 依次压入1/2/3, 必须依次弹出3/2/1
 */
TEST_CASE("pop follows last in first out order") {
    Stack stack = make_stack(4, {1, 2, 3});

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, stack_pop(&stack, &value));
    CHECK_EQ(value, 3);
    CHECK_EQ(TOYLIB_OK, stack_pop(&stack, &value));
    CHECK_EQ(value, 2);
    CHECK_EQ(TOYLIB_OK, stack_pop(&stack, &value));
    CHECK_EQ(value, 1);

    CHECK_EQ(stack.length_, 0);
    stack_free(&stack);
}

/**
 * 校验length返回当前元素个数
 * @note 覆盖空栈、压入之后、弹出之后三个状态
 */
TEST_CASE("length reports the element count") {
    Stack stack = make_stack(4, {});

    size_t length = 99;
    CHECK_EQ(TOYLIB_OK, stack_length(&stack, &length));
    CHECK_EQ(length, 0);

    CHECK_EQ(TOYLIB_OK, stack_push(&stack, 1));
    CHECK_EQ(TOYLIB_OK, stack_push(&stack, 2));
    CHECK_EQ(TOYLIB_OK, stack_length(&stack, &length));
    CHECK_EQ(length, 2);

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, stack_pop(&stack, &value));
    CHECK_EQ(TOYLIB_OK, stack_length(&stack, &length));
    CHECK_EQ(length, 1);

    stack_free(&stack);
}

/**
 * 校验free释放内存并把三个字段复位
 * @note 复位之后才能被未初始化检查识别, 见L2的free rejects a stack that was already freed
 */
TEST_CASE("free releases memory and resets the fields") {
    Stack stack = make_stack(4, {1, 2, 3});

    CHECK_EQ(TOYLIB_OK, stack_free(&stack));

    CHECK(stack.data_ == nullptr);
    CHECK_EQ(stack.capacity_, 0);
    CHECK_EQ(stack.length_, 0);
}

TEST_SUITE_END();
