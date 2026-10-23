/*
 * L2边界级, 覆盖参数校验与容量边界, 并验证操作失败后状态不变
 */

#include <cstdint>

#include "doctest/doctest.h"
#include "queue_support.h"

using toylib_test::make_queue;
using toylib_test::Value;

TEST_SUITE_BEGIN("[Queue][L2]");

/**
 * 校验init拒绝空指针, 返回BAD_ARG_NULL
 */
TEST_CASE("init rejects a null queue pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_init(nullptr, 4));
}

/**
 * 校验init校验容量下界: 0不是合法容量
 * @note 容量下界封闭, 1是合法的最小值, 和seq_list_create/stack_init的判定保持一致
 */
TEST_CASE("init rejects a zero capacity") {
    // 用哨兵值打底: 起手全零的话, 实现"写0"和"压根不写"在断言上看不出区别
    Queue queue{7, 3, 5, nullptr};

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_VALUE, queue_init(&queue, 0));

    // 失败不能留下半初始化的状态, 也不能把调用方的原有内容改掉
    CHECK_EQ(queue.capacity_, 7);
    CHECK_EQ(queue.front_, 3);
    CHECK_EQ(queue.length_, 5);
    CHECK(queue.data_ == nullptr);
}

/**
 * 校验init校验容量上界: 超过MAX_SIZE都要拒绝
 * @note 上界是闭区间, MAX_SIZE本身合法, 加一就不行
 */
TEST_CASE("init rejects a capacity above the max") {
    Queue queue{7, 3, 5, nullptr};

    SUBCASE("capacity is max plus one") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_VALUE, queue_init(&queue, TOYLIB_QUEUE_MAX_SIZE + 1));
    }
    SUBCASE("capacity is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_VALUE, queue_init(&queue, SIZE_MAX));
    }

    CHECK_EQ(queue.capacity_, 7);
    CHECK_EQ(queue.front_, 3);
    CHECK_EQ(queue.length_, 5);
    CHECK(queue.data_ == nullptr);
}

/**
 * 校验push拒绝空指针, 返回BAD_ARG_NULL
 */
TEST_CASE("push rejects a null queue pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_push(nullptr, 1));
}

/**
 * 校验满队列时push返回CAPACITY_FULL, 且不破坏已有内容
 * @note 容量取1, 让"满"和"空"只差一个元素, 更容易看出长度有没有被改动
 */
TEST_CASE("push on a full queue reports capacity full") {
    Queue queue = make_queue(1, {7});

    CHECK_EQ(TOYLIB_ERR_CAPACITY_FULL, queue_push(&queue, 8));

    CHECK_EQ(queue.length_, 1);
    CHECK_QUEUE(queue, 7);
    queue_free(&queue);
}

/**
 * 校验容量只在出队之后才腾出来, 出队后能继续入队
 * @note push被拒时不该动length_, 否则这个"弹一个再压一个"的流程会错位
 */
TEST_CASE("push succeeds again after a pop frees a slot") {
    Queue queue = make_queue(1, {7});

    CHECK_EQ(TOYLIB_ERR_CAPACITY_FULL, queue_push(&queue, 8));

    Value value = 0;
    REQUIRE_EQ(TOYLIB_OK, queue_pop(&queue, &value));
    CHECK_EQ(value, 7);

    CHECK_EQ(TOYLIB_OK, queue_push(&queue, 9));
    CHECK_EQ(queue.length_, 1);
    CHECK_QUEUE(queue, 9);
    queue_free(&queue);
}

/**
 * 校验满队列被反复撞满也不会让front_和length_漂移
 * @note 队尾位置是推出来的, 唯一能漂的就是front_和length_;
 *       被拒的push如果顺手改了长度, 后面算出来的队尾就全错了
 */
TEST_CASE("rejected push leaves the queue untouched") {
    Queue queue = make_queue(2, {1, 2});

    const size_t front = queue.front_;
    for (int i = 0; i < 5; ++i)
        CHECK_EQ(TOYLIB_ERR_CAPACITY_FULL, queue_push(&queue, 9));

    CHECK_EQ(queue.front_, front);
    CHECK_EQ(queue.length_, 2);
    CHECK_QUEUE(queue, 1, 2);
    queue_free(&queue);
}

/**
 * 校验空队列pop返回EMPTY, 出参和长度都保持原样
 * @note 读不到元素时不能往调用方的变量里写东西
 */
TEST_CASE("pop on an empty queue reports empty") {
    Queue queue = make_queue(4, {});

    Value value = 42;
    CHECK_EQ(TOYLIB_ERR_EMPTY, queue_pop(&queue, &value));

    CHECK_EQ(value, 42);
    CHECK_EQ(queue.length_, 0);
    CHECK_EQ(queue.front_, 0);
    queue_free(&queue);
}

/**
 * 校验空队列front返回EMPTY, 出参保持原样
 * @note 空队列的front_仍然指向0, 直接取值会读到未初始化的槽
 */
TEST_CASE("front on an empty queue reports empty") {
    Queue queue = make_queue(4, {});

    Value value = 42;
    CHECK_EQ(TOYLIB_ERR_EMPTY, queue_front(&queue, &value));

    CHECK_EQ(value, 42);
    CHECK_EQ(queue.length_, 0);
    queue_free(&queue);
}

/**
 * 校验front拒绝空出参, 返回参数空指针错误
 * @note 出参是唯一的返回值载体, 为空就没地方写结果
 */
TEST_CASE("front rejects a null out parameter") {
    Queue queue = make_queue(4, {1});

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_front(&queue, nullptr));

    CHECK_EQ(queue.length_, 1);
    CHECK_QUEUE(queue, 1);
    queue_free(&queue);
}

/**
 * 校验pop允许空出参, 此时只弹出不取值
 * @note 和seq_list_delete/dlist_delete/stack_pop的出参约定一致: 为空指针则只做动作
 */
TEST_CASE("pop accepts a null out parameter") {
    Queue queue = make_queue(4, {1, 2});

    CHECK_EQ(TOYLIB_OK, queue_pop(&queue, nullptr));

    CHECK_EQ(queue.length_, 1);
    CHECK_QUEUE(queue, 2);
    queue_free(&queue);
}

/**
 * 校验pop和front拒绝空队列指针
 * @note 空指针检查和对出参的空指针检查是两个独立条件, 要分开测;
 *       只测"出参为空"的话, "队列指针为空"那条分支永远走不到
 */
TEST_CASE("pop and front reject a null queue pointer") {
    SUBCASE("pop") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_pop(nullptr, &value));
    }
    SUBCASE("front") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_front(nullptr, &value));
    }
    SUBCASE("both arguments are null") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_pop(nullptr, nullptr));
    }
    SUBCASE("front with both arguments null") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_front(nullptr, nullptr));
    }
}

/**
 * 校验未初始化的队列上所有操作都返回NOT_INITIALIZED
 * @note 零初始化的队列data_为空, 应该被识别出来, 而不是让空指针参与下标运算;
 *       pop这里必须传有效出参, 否则会先撞上"出参为空"的检查
 */
TEST_CASE("operations on an uninitialized queue report not initialized") {
    Queue queue{};

    SUBCASE("push") {
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_push(&queue, 1));
    }
    SUBCASE("pop") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_pop(&queue, &value));
    }
    SUBCASE("front") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_front(&queue, &value));
    }
    SUBCASE("free") {
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_free(&queue));
    }
    SUBCASE("length") {
        size_t length = 99;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_length(&queue, &length));
        // 出错时不准碰出参, 否则调用方分不清拿到的是结果还是自己的初值
        CHECK_EQ(length, 99);
    }
    SUBCASE("capacity") {
        size_t capacity = 99;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_capacity(&queue, &capacity));
        CHECK_EQ(capacity, 99);
    }
}

/**
 * 校验length和capacity拒绝空指针与空出参
 * @note 出参是唯一的返回值载体, 为空就没地方写结果
 */
TEST_CASE("length and capacity reject invalid arguments") {
    SUBCASE("length rejects a null queue pointer") {
        size_t length = 0;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_length(nullptr, &length));
    }
    SUBCASE("length rejects a null out parameter") {
        Queue queue = make_queue(4, {});
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_length(&queue, nullptr));
        queue_free(&queue);
    }
    SUBCASE("capacity rejects a null queue pointer") {
        size_t capacity = 0;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_capacity(nullptr, &capacity));
    }
    SUBCASE("capacity rejects a null out parameter") {
        Queue queue = make_queue(4, {});
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_capacity(&queue, nullptr));
        queue_free(&queue);
    }
}

/**
 * 校验free拒绝空指针
 */
TEST_CASE("free rejects a null queue pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, queue_free(nullptr));
}

/**
 * 校验重复free被未初始化检查拦下
 * @note 第一次free必须把data_置空, 否则第二次会去free一个悬空指针
 */
TEST_CASE("free rejects a queue that was already freed") {
    Queue queue = make_queue(4, {1, 2, 3});

    CHECK_EQ(TOYLIB_OK, queue_free(&queue));
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_free(&queue));
}

/**
 * 校验free之后其余操作全部退回NOT_INITIALIZED
 * @note 零初始化(从未init)和"init过后又free"是两条不同的路径, 内存状态都是全零,
 *       但只有后者验证了free确实把结构体复位干净, 不会让后续操作拿到悬空指针
 */
TEST_CASE("operations on a freed queue report not initialized") {
    Queue queue = make_queue(4, {1, 2, 3});
    REQUIRE_EQ(TOYLIB_OK, queue_free(&queue));

    SUBCASE("push") {
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_push(&queue, 1));
    }
    SUBCASE("pop") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_pop(&queue, &value));
    }
    SUBCASE("front") {
        Value value = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_front(&queue, &value));
    }
    SUBCASE("length") {
        size_t length = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_length(&queue, &length));
    }
    SUBCASE("capacity") {
        size_t capacity = 0;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, queue_capacity(&queue, &capacity));
    }
}

TEST_SUITE_END();
