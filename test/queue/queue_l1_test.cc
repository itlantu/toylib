/*
 * L1契约级, 锁定每个API的最小可用路径和后置状态
 *
 * 队列比栈和顺序表多一层结构: 元素不再总是从data_[0]开始,
 * 所以除了"值对不对", 还要锁住front_/rear_的走向和回绕规则。
 */

#include "doctest/doctest.h"
#include "queue_support.h"

using toylib_test::make_queue;
using toylib_test::Value;

TEST_SUITE_BEGIN("[Queue][L1]");

/**
 * 校验init按容量分配内存, 建出一个空队列
 * @note 契约: capacity_等于入参, length_为0, front_和rear_都归0, data_指向有效内存
 */
TEST_CASE("init allocates memory and leaves the queue empty") {
    Queue queue{};

    REQUIRE_EQ(TOYLIB_OK, queue_init(&queue, 4));

    CHECK_EQ(queue.capacity_, 4);
    CHECK_EQ(queue.length_, 0);
    CHECK_EQ(queue.front_, 0);
    CHECK_EQ(queue.rear_, 0);
    CHECK(queue.data_ != nullptr);

    queue_free(&queue);
}

/**
 * 校验init接受合法的最小容量 1
 * @note 容量下界封闭: 1是合法的最小值, 0才算非法
 */
TEST_CASE("init accepts the minimal capacity of 1") {
    Queue queue{};
    REQUIRE_EQ(TOYLIB_OK, queue_init(&queue, 1));

    CHECK_EQ(queue.capacity_, 1);
    CHECK_EQ(queue.length_, 0);

    queue_free(&queue);
}

/**
 * 校验push把值放到队尾, 长度只增加一个
 * @note 契约: 容量由init决定, 入队过程中不该变化
 */
TEST_CASE("push appends to the rear and grows the queue by one") {
    Queue queue = make_queue(4, {});

    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 10));
    CHECK_EQ(queue.length_, 1);
    CHECK_EQ(queue.capacity_, 4);

    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 20));
    CHECK_EQ(queue.length_, 2);

    CHECK_QUEUE(queue, 10, 20);
    queue_free(&queue);
}

/**
 * 校验front返回队头的值
 * @note 队头是入队最早且尚未弹出的元素, 不是最后进来的那个
 */
TEST_CASE("front returns the head value") {
    Queue queue = make_queue(4, {1, 2, 3});

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, queue_front(&queue, &value));

    CHECK_EQ(value, 1);
    queue_free(&queue);
}

/**
 * 校验rear返回队尾的值
 * @note rear_指向下一个可写位置, 所以队尾元素落在rear_的前一格(要回绕)
 */
TEST_CASE("rear returns the tail value") {
    Queue queue = make_queue(4, {1, 2, 3});

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, queue_rear(&queue, &value));

    CHECK_EQ(value, 3);
    queue_free(&queue);
}

/**
 * 校验front和rear都是只读操作, 不弹出也不改动队列
 * @note 连续读两次结果相同, 长度和内容都不该变
 */
TEST_CASE("front and rear do not modify the queue") {
    Queue queue = make_queue(4, {1, 2, 3});

    Value first_front = 0;
    Value second_front = 0;
    Value first_rear = 0;
    Value second_rear = 0;
    CHECK_EQ(TOYLIB_OK, queue_front(&queue, &first_front));
    CHECK_EQ(TOYLIB_OK, queue_front(&queue, &second_front));
    CHECK_EQ(TOYLIB_OK, queue_rear(&queue, &first_rear));
    CHECK_EQ(TOYLIB_OK, queue_rear(&queue, &second_rear));

    CHECK_EQ(first_front, second_front);
    CHECK_EQ(first_rear, second_rear);
    CHECK_EQ(queue.length_, 3);
    CHECK_QUEUE(queue, 1, 2, 3);
    queue_free(&queue);
}

/**
 * 校验pop带回队头的值, 并把长度减一
 * @note 出队后剩下的还是原来那些元素, 只是少了队头一个
 */
TEST_CASE("pop returns the head value and shrinks the queue") {
    Queue queue = make_queue(4, {1, 2, 3});

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));

    CHECK_EQ(value, 1);
    CHECK_EQ(queue.length_, 2);
    CHECK_QUEUE(queue, 2, 3);
    queue_free(&queue);
}

/**
 * 校验pop严格按先进先出顺序吐出元素
 * @note 依次入队1/2/3, 必须依次弹出1/2/3
 */
TEST_CASE("pop follows first in first out order") {
    Queue queue = make_queue(4, {1, 2, 3});

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(value, 1);
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(value, 2);
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(value, 3);

    CHECK_EQ(queue.length_, 0);
    queue_free(&queue);
}

/**
 * 校验出队腾出的空间能被后续入队复用, 下标在数组末尾回绕
 * @note 这是循环队列的核心契约: 容量3的队列反复进出, 元素会跨过data_末尾;
 *       如果实现靠"整体前移"来省空间, 这里的内容和front_都对不上
 */
TEST_CASE("push reuses the slot freed by pop and wraps around") {
    Queue queue = make_queue(3, {1, 2, 3});

    // 先出两个, front_走到2, 前面0和1两个槽空出来
    Value value = 0;
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(queue.front_, 2);

    // 再入两个: 第二个会写到data_[0], 也就是数组末尾回绕到开头
    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 4));
    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 5));

    CHECK_EQ(queue.length_, 3);
    CHECK_EQ(queue.front_, 2);
    CHECK_EQ(queue.rear_, 2);
    // 物理上跨了末尾, 逻辑上仍然是连续的三元素
    CHECK_EQ(queue.data_[2], 3);
    CHECK_EQ(queue.data_[0], 4);
    CHECK_EQ(queue.data_[1], 5);
    CHECK_QUEUE(queue, 3, 4, 5);
    queue_free(&queue);
}

/**
 * 校验填满到容量之后, 队头和队尾分别是最早和最晚入队的值
 * @note length_等于capacity_时front_和rear_相等, 只能靠length_区分"满"和"空"
 */
TEST_CASE("front and rear stay correct when the queue is full") {
    Queue queue = make_queue(3, {1, 2, 3});

    CHECK_EQ(queue.length_, queue.capacity_);
    CHECK_EQ(queue.front_, queue.rear_);

    Value front = 0;
    Value rear = 0;
    CHECK_EQ(TOYLIB_OK, queue_front(&queue, &front));
    CHECK_EQ(TOYLIB_OK, queue_rear(&queue, &rear));

    CHECK_EQ(front, 1);
    CHECK_EQ(rear, 3);
    queue_free(&queue);
}

/**
 * 校验length返回当前元素个数
 * @note 覆盖空队列、入队之后、出队之后三个状态
 */
TEST_CASE("length reports the element count") {
    Queue queue = make_queue(4, {});

    size_t length = 99;
    CHECK_EQ(TOYLIB_OK, queue_length(&queue, &length));
    CHECK_EQ(length, 0);

    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 1));
    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 2));
    CHECK_EQ(TOYLIB_OK, queue_length(&queue, &length));
    CHECK_EQ(length, 2);

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(TOYLIB_OK, queue_length(&queue, &length));
    CHECK_EQ(length, 1);

    queue_free(&queue);
}

/**
 * 校验capacity返回init时确定的容量
 * @note 容量只在init时确定, 入队出队都不会改变它
 */
TEST_CASE("capacity reports the allocated capacity") {
    Queue queue = make_queue(8, {1, 2, 3});

    size_t capacity = 99;
    CHECK_EQ(TOYLIB_OK, queue_capacity(&queue, &capacity));
    CHECK_EQ(capacity, 8);

    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 4));
    CHECK_EQ(TOYLIB_OK, queue_capacity(&queue, &capacity));
    CHECK_EQ(capacity, 8);

    Value value = 0;
    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(TOYLIB_OK, queue_capacity(&queue, &capacity));
    CHECK_EQ(capacity, 8);

    queue_free(&queue);
}

/**
 * 校验free释放内存并把五个字段复位
 * @note 复位之后才能被未初始化检查识别, 见L2的free rejects a queue that was already freed
 */
TEST_CASE("free releases memory and resets the fields") {
    Queue queue = make_queue(4, {1, 2, 3});

    CHECK_EQ(TOYLIB_OK, queue_free(&queue));

    CHECK(queue.data_ == nullptr);
    CHECK_EQ(queue.capacity_, 0);
    CHECK_EQ(queue.length_, 0);
    CHECK_EQ(queue.front_, 0);
    CHECK_EQ(queue.rear_, 0);
}

TEST_SUITE_END();
