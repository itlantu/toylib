/*
 * Queue测试的辅助函数
 *
 * 只服务于test/queue目录下的用例, 不做跨模块复用
 */

#ifndef TOYLIB_TEST_QUEUE_SUPPORT_H
#define TOYLIB_TEST_QUEUE_SUPPORT_H

#include <initializer_list>
#include <vector>

#include "doctest/doctest.h"
#include "test_support.h"
#include "toylib/queue.h"

namespace toylib_test {

using Value = TOYLIB_QUEUE_VALUE_TYPE;

/**
 * L3随机序列测试的操作名, 下标就是用例里的op编号
 * @note 顺序必须和L3的switch保持一致; 长度顺便充当可抽取的操作数
 */
inline constexpr const char* kOpNames[] = {"push", "pop", "front", "length"};

/**
 * 从队头到队尾收集元素
 * @param queue 目标队列
 * @return 元素拷贝, 下标0是队头, 末尾是队尾
 * @note 循环队列的元素可以跨过数组末尾, data_不再是连续区间,
 *       所以必须从front_出发按capacity_回绕逐个读;
 *       这里不额外判capacity_为0: 长度大于0时容量必然大于0, 长度本身就挡住了循环,
 *       真出现"容量0但长度非0"说明不变量已经坏了, 宁可当场崩掉也不要静默返回空表
 */
inline std::vector<Value> to_values(const Queue& queue) {
    std::vector<Value> values;
    values.reserve(queue.length_);
    for (size_t i = 0; i < queue.length_; ++i)
        values.push_back(queue.data_[(queue.front_ + i) % queue.capacity_]);
    return values;
}

/**
 * 建一个队列并按顺序入队初值
 * @param capacity 队列容量
 * @param values 初始元素, 第一个是队头, 也就是最先被弹出的那个
 * @return 初始化好的队列, 用完需要queue_free
 * @note 内部不检查返回值, 只适合容量和元素个数都合法的场景
 */
inline Queue make_queue(const size_t capacity, const std::initializer_list<Value> values) {
    Queue queue{};
    queue_init(&queue, capacity);
    for (const Value value : values)
        queue_push(&queue, value);
    return queue;
}

/**
 * 建一个字段全是哨兵值的队列, 用来确认失败的init不会改写调用方的内存
 * @return 四个字段都带非0初值的未初始化队列
 * @note 起手全零的话, 实现"写0"和"压根不写"在断言上看不出区别
 */
inline Queue make_sentinel_queue() {
    return Queue{7, 3, 5, nullptr};
}

/**
 * 断言队列的四个字段还停在make_sentinel_queue给的哨兵值上
 * @param queue 检查目标, 必须是make_sentinel_queue造出来的队列
 */
inline void check_sentinel_queue(const Queue& queue) {
    CHECK_EQ(queue.capacity_, 7);
    CHECK_EQ(queue.front_, 3);
    CHECK_EQ(queue.length_, 5);
    CHECK(queue.data_ == nullptr);
}

}  // namespace toylib_test

// 断言队列从队头到队尾和期望一致, 例如 CHECK_QUEUE(queue, 1, 2, 3)
#define CHECK_QUEUE(queue, ...) CHECK_EQ(toylib_test::to_values(queue), (std::vector<toylib_test::Value>{__VA_ARGS__}))

#endif  // TOYLIB_TEST_QUEUE_SUPPORT_H
