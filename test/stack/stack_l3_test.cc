/*
 * L3场景级, 用std::vector当参考模型, 跑随机操作序列逐步比对
 *
 * 栈的状态只有length_一条线索, 单步测试看不出组合问题;
 * 随机序列能把"压满再弹""弹空再压""在满和空之间反复来回"这些路径都走一遍。
 */

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "stack_support.h"

using toylib_test::make_stack;
using toylib_test::Value;

TEST_SUITE_BEGIN("[Stack][L3]");

/**
 * 校验随机操作序列下, 栈的行为和参考模型逐步一致
 * @note 种子固定, 失败时按step就能复现; 改种子可以探索另一条路径
 */
TEST_CASE("random operations match the model") {
    constexpr int kSteps = 500;
    constexpr size_t kCapacity = 8;
    constexpr unsigned kSeed = 20261022u;

    std::mt19937 rng(kSeed);
    std::vector<Value> model;

    Stack stack = make_stack(kCapacity, {});

    for (int step = 0; step < kSteps; ++step) {
        // push和pop各占4成, 让长度在0和容量之间来回走, 更容易撞到满栈和空栈
        const unsigned dice = rng() % 10;
        const int op = dice < 4 ? 0 : (dice < 8 ? 1 : (dice == 8 ? 2 : 3));
        const Value value = static_cast<Value>(rng() % 100);

        std::string op_name = "push";
        if (op == 1)
            op_name = "pop";
        else if (op == 2)
            op_name = "top";
        else if (op == 3)
            op_name = "length";
        CAPTURE(step);
        CAPTURE(op_name);
        CAPTURE(value);

        switch (op) {
            case 0: {
                // push: 唯一的上界是容量
                const ToylibError expected = model.size() == kCapacity ? TOYLIB_ERR_CAPACITY_FULL : TOYLIB_OK;

                CHECK_EQ(expected, stack_push(&stack, value));
                if (expected == TOYLIB_OK)
                    model.push_back(value);
                break;
            }
            case 1: {
                // pop: 空栈返回EMPTY, 否则带回原栈顶
                const ToylibError expected = model.empty() ? TOYLIB_ERR_EMPTY : TOYLIB_OK;

                Value popped = 0;
                CHECK_EQ(expected, stack_pop(&stack, &popped));
                if (expected == TOYLIB_OK) {
                    CHECK_EQ(popped, model.back());
                    model.pop_back();
                }
                break;
            }
            case 2: {
                // top: 只读操作, 空栈返回EMPTY
                const ToylibError expected = model.empty() ? TOYLIB_ERR_EMPTY : TOYLIB_OK;

                Value top = 0;
                CHECK_EQ(expected, stack_top(&stack, &top));
                if (expected == TOYLIB_OK)
                    CHECK_EQ(top, model.back());
                break;
            }
            default: {
                size_t length = 0;
                CHECK_EQ(TOYLIB_OK, stack_length(&stack, &length));
                CHECK_EQ(length, model.size());
                break;
            }
        }

        // 每一步之后全量比对, 从栈底到栈顶
        CHECK_EQ(toylib_test::to_values(stack), model);
    }

    stack_free(&stack);
}

TEST_SUITE_END();
