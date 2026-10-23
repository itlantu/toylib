/*
 * L3场景级, 用std::deque当参考模型, 跑随机操作序列逐步比对
 *
 * 队列的状态只有两条线索: 内容顺序和length_, 队尾位置是从这两条推出来的。
 * 单步测试只能覆盖其中一条, 随机序列能把"填满再弹出""弹空再入队"
 * "在满和空之间反复来回""下标反复回绕"这些组合路径都走一遍。
 */

#include <cstdint>
#include <deque>
#include <random>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "queue_support.h"

using toylib_test::make_queue;
using toylib_test::to_values;
using toylib_test::Value;

TEST_SUITE_BEGIN("[Queue][L3]");

/**
 * 校验随机操作序列下, 队列的行为和参考模型逐步一致
 * @note 种子固定, 失败时按step就能复现; 改种子可以探索另一条路径
 */
TEST_CASE("random operations match the model") {
    constexpr int kSteps = 500;
    constexpr size_t kCapacity = 8;
    constexpr unsigned kSeed = 20261023u;

    std::mt19937 rng(kSeed);
    std::deque<Value> model;

    Queue queue = make_queue(kCapacity, {});

    for (int step = 0; step < kSteps; ++step) {
        // push占4成, pop占3成, 让长度在0和容量之间来回走, 更容易撞到满和空
        const unsigned dice = rng() % 10;
        const int op = dice < 4 ? 0 : (dice < 7 ? 1 : (dice < 9 ? 2 : 3));
        const Value value = static_cast<Value>(rng() % 100);

        std::string op_name = "push";
        if (op == 1)
            op_name = "pop";
        else if (op == 2)
            op_name = "front";
        else if (op == 3)
            op_name = "length";
        CAPTURE(step);
        CAPTURE(op_name);
        CAPTURE(value);

        switch (op) {
            case 0: {
                // push: 唯一的上界是容量
                const ToylibError expected = model.size() == kCapacity ? TOYLIB_ERR_CAPACITY_FULL : TOYLIB_OK;

                CHECK_EQ(expected, queue_push(&queue, value));
                if (expected == TOYLIB_OK)
                    model.push_back(value);
                break;
            }
            case 1: {
                // pop: 空队列返回EMPTY, 否则带回原队头
                const ToylibError expected = model.empty() ? TOYLIB_ERR_EMPTY : TOYLIB_OK;

                Value popped = 0;
                CHECK_EQ(expected, queue_pop(&queue, &popped));
                if (expected == TOYLIB_OK) {
                    CHECK_EQ(popped, model.front());
                    model.pop_front();
                }
                break;
            }
            case 2: {
                // front: 只读操作, 空队列返回EMPTY
                const ToylibError expected = model.empty() ? TOYLIB_ERR_EMPTY : TOYLIB_OK;

                Value front = 0;
                CHECK_EQ(expected, queue_front(&queue, &front));
                if (expected == TOYLIB_OK)
                    CHECK_EQ(front, model.front());
                break;
            }
            default: {
                size_t length = 0;
                CHECK_EQ(TOYLIB_OK, queue_length(&queue, &length));
                CHECK_EQ(length, model.size());
                break;
            }
        }

        // 每一步之后全量比对, 从队头到队尾
        CHECK_EQ(to_values(queue), std::vector<Value>(model.begin(), model.end()));

        // 结构不变量: 下标必须始终落在[0, capacity), 长度不能超过容量
        CHECK(queue.front_ < queue.capacity_);
        CHECK(queue.length_ <= queue.capacity_);
    }

    queue_free(&queue);
}

/**
 * 校验队头下标靠回绕前进, 而不是靠搬移元素维持
 * @note 队列始终保持满: 每轮弹一个再补一个, 于是front_每轮后移一格,
 *       绕着data_转满一整圈。如果实现靠"整体前移"来让front_回到0,
 *       下面这条front_的断言立刻就对不上了
 */
TEST_CASE("the head index wraps around while the queue stays full") {
    constexpr size_t kCapacity = 4;
    constexpr int kRounds = 12;

    Queue queue = make_queue(kCapacity, {});
    std::deque<Value> model;
    Value next = 0;

    for (int round = 0; round < kRounds; ++round) {
        CAPTURE(round);

        // 补到满
        while (model.size() < kCapacity) {
            REQUIRE_EQ(TOYLIB_OK, queue_push(&queue, next));
            model.push_back(next);
            next = static_cast<Value>(next + 1);
        }
        CHECK_EQ(queue.length_, kCapacity);

        // 弹一个再补一个, 空出来的槽立刻被复用
        Value value = 0;
        REQUIRE_EQ(TOYLIB_OK, queue_pop(&queue, &value));
        CHECK_EQ(value, model.front());
        model.pop_front();

        REQUIRE_EQ(TOYLIB_OK, queue_push(&queue, next));
        model.push_back(next);
        next = static_cast<Value>(next + 1);

        CHECK_EQ(queue.length_, kCapacity);
        CHECK_EQ(queue.front_, (static_cast<size_t>(round) + 1) % kCapacity);
        CHECK_EQ(to_values(queue), std::vector<Value>(model.begin(), model.end()));
    }

    queue_free(&queue);
}

TEST_SUITE_END();
