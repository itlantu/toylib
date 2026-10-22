/*
 * L2边界级, 覆盖参数校验、容量与下标的边界值, 并验证操作失败后状态不变
 * 约定:
 *  - 参数为空指针返回 BAD_ARG_NULL, 参数值非法返回 BAD_ARG_VALUE
 *  - insert 同时不满足下标和容量时, 先报下标错误
 *  - 任何失败都不能改动 length_ / capacity_ / 已有元素
 */

#include <cstdint>

#include "doctest/doctest.h"
#include "seqlist_support.h"

using toylib_test::check_create_rejected;
using toylib_test::check_init_rejected;
using toylib_test::Element;
using toylib_test::make_list;
using toylib_test::to_vector;

TEST_SUITE_BEGIN("[SeqList][L2]");

/**
 * 校验create拒绝空指针, 返回参数空指针错误
 * @note 空指针检查必须是函数入口的第一件事, 不能先解引用
 */
TEST_CASE("create rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_create(nullptr, 8));
}

/**
 * 校验 create拒绝 0 / 上限加一 / SIZE_MAX 三种非法容量
 * @note 契约: 返回BAD_ARG_2, 且不留下半初始化的字段
 */
TEST_CASE("create rejects an invalid capacity") {
    SUBCASE("capacity is zero") {
        check_create_rejected(TOYLIB_ERR_BAD_ARG_VALUE, 0);
    }
    SUBCASE("capacity is max plus one") {
        check_create_rejected(TOYLIB_ERR_BAD_ARG_VALUE, TOYLIB_SEQLIST_MAX_CAPACITY + 1);
    }
    SUBCASE("capacity is SIZE_MAX") {
        check_create_rejected(TOYLIB_ERR_BAD_ARG_VALUE, SIZE_MAX);
    }
}

/**
 * 校验容量上界 MAX_CAPACITY 本身是合法的
 */
TEST_CASE("create accepts the maximum capacity") {
    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_create(&list, TOYLIB_SEQLIST_MAX_CAPACITY));

    CHECK(list.data_ != nullptr);
    CHECK_EQ(list.capacity_, TOYLIB_SEQLIST_MAX_CAPACITY);

    seq_list_free(&list);
}

/**
 * 校验init对每个非法参数返回对应的错误码
 */
TEST_CASE("init rejects invalid arguments") {
    int buffer[4] = {1, 2, 3, 4};
    SeqList list{};

    SUBCASE("null list pointer") {
        check_init_rejected(TOYLIB_ERR_BAD_ARG_NULL, nullptr, buffer, 4);
    }
    SUBCASE("null data pointer") {
        check_init_rejected(TOYLIB_ERR_BAD_ARG_NULL, &list, nullptr, 4);
    }
    SUBCASE("capacity is zero") {
        check_init_rejected(TOYLIB_ERR_BAD_ARG_VALUE, &list, buffer, 0);
    }
    SUBCASE("capacity is above the limit") {
        check_init_rejected(TOYLIB_ERR_BAD_ARG_VALUE, &list, buffer, TOYLIB_SEQLIST_MAX_CAPACITY + 1);
    }
}

/**
 * 校验insert拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("insert rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_insert(nullptr, 0, 1));
}

/**
 * 校验插入位置的上界: 长度加一和 SIZE_MAX 都要拒绝
 * @note 插入位置的合法区间是[0, length]
 */
TEST_CASE("insert rejects an index past the end") {
    SeqList list = make_list(8, {1, 2});
    const auto before = to_vector(list);

    SUBCASE("index is length plus one") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_insert(&list, 3, 9));
    }
    SUBCASE("index is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_insert(&list, SIZE_MAX, 9));
    }

    // 失败后状态不变
    CHECK_EQ(to_vector(list), before);
    seq_list_free(&list);
}

/**
 * 校验满表插入的错误码, 以及下标与容量的判定顺序
 * @note 下标取 length 时报容量已满; 下标越界时报下标错误, 即下标优先
 */
TEST_CASE("insert on a full list reports the right error") {
    SeqList list = make_list(4, {1, 2, 3, 4});
    const auto before = to_vector(list);

    SUBCASE("index equals length reports capacity full") {
        CHECK_EQ(TOYLIB_ERR_CAPACITY_FULL, seq_list_insert(&list, 4, 5));
    }
    SUBCASE("index past the end reports out of range") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_insert(&list, 5, 5));
    }

    CHECK_EQ(to_vector(list), before);
    seq_list_free(&list);
}

/**
 * 校验delete拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("delete rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_delete(nullptr, 0, nullptr));
}

/**
 * 校验空表删除返回EMPTY
 */
TEST_CASE("delete on an empty list reports empty") {
    SeqList list = make_list(4, {});

    int removed = 0;
    CHECK_EQ(TOYLIB_ERR_EMPTY, seq_list_delete(&list, 0, &removed));
    CHECK_EQ(removed, 0);

    seq_list_free(&list);
}

/**
 * 校验删除位置的上界: 长度和SIZE_MAX都要拒绝
 * @note 删除位置的合法区间是[0, length), 右端是开区间
 */
TEST_CASE("delete rejects an index at or past the end") {
    SeqList list = make_list(8, {1, 2, 3});
    const auto before = to_vector(list);

    SUBCASE("index equals length") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_delete(&list, 3, nullptr));
    }
    SUBCASE("index is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_delete(&list, SIZE_MAX, nullptr));
    }

    CHECK_EQ(to_vector(list), before);
    seq_list_free(&list);
}

/**
 * 校验删除首个元素时后面的元素全部前移
 * @note 这是删除位置的下界0
 */
TEST_CASE("delete at index 0 shifts every element left") {
    SeqList list = make_list(16, {1, 2, 3, 4, 5});

    int removed = 0;
    CHECK_EQ(TOYLIB_OK, seq_list_delete(&list, 0, &removed));

    CHECK_EQ(removed, 1);
    CHECK_SEQ(list, 2, 3, 4, 5);
    seq_list_free(&list);
}

/**
 * 校验满表删除最后一个元素
 * @note 此时length_等于capacity_, 位移循环不能多走一步写到容量之外
 */
TEST_CASE("delete the last element of a full list") {
    SeqList list = make_list(4, {1, 2, 3, 4});

    int removed = 0;
    CHECK_EQ(TOYLIB_OK, seq_list_delete(&list, 3, &removed));

    CHECK_EQ(removed, 4);
    CHECK_SEQ(list, 1, 2, 3);
    seq_list_free(&list);
}

/**
 * 校验delete不会写到容量之外
 * @note 用init借用一段后面带哨兵的数组, 越界写会踩坏哨兵, 比等malloc/free报堆损坏稳定
 */
TEST_CASE("delete never writes past the capacity") {
    // 哨兵值不能相同, 否则越界复制可能把同一个值搬回来, 反而看不出来
    int buffer[8] = {1, 2, 3, 4, 11, 22, 33, 44};

    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_init(&list, buffer, 4));
    for (size_t i = 0; i < 4; ++i) {
        REQUIRE_EQ(TOYLIB_OK, seq_list_insert(&list, i, static_cast<Element>(i + 1)));
    }

    // 容量之后一格都不该被碰
    CHECK_EQ(buffer[4], 11);
    CHECK_EQ(buffer[5], 22);
    CHECK_EQ(buffer[6], 33);

    int removed = 0;
    CHECK_EQ(TOYLIB_OK, seq_list_delete(&list, 3, &removed));

    CHECK_EQ(removed, 4);
    CHECK_SEQ(list, 1, 2, 3);
    // 容量之后一格都不该被碰
    CHECK_EQ(buffer[4], 11);
    CHECK_EQ(buffer[5], 22);
    CHECK_EQ(buffer[6], 33);
}

/**
 * 校验find拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("find rejects a null list pointer") {
    size_t index = 0;
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_find(nullptr, &index, 1));
}

/**
 * 校验find拒绝空的出参, 返回BAD_ARG_2
 * @note 没地方写下标就没法带回结果, 属于调用方的错误
 */
TEST_CASE("find rejects a null index out parameter") {
    SeqList list = make_list(8, {1, 2, 3});

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_find(&list, nullptr, 2));
    seq_list_free(&list);
}

/**
 * 校验空表里查不到任何元素
 * @note 查不到时index出参写成哨兵值TOYLIB_SEQLIST_ELEMENT_NOT_FOUND
 */
TEST_CASE("find on an empty list reports not found") {
    SeqList list = make_list(4, {});
    size_t index = 99;

    CHECK_EQ(TOYLIB_OK, seq_list_find(&list, &index, 1));
    CHECK_EQ(index, TOYLIB_SEQLIST_ELEMENT_NOT_FOUND);

    seq_list_free(&list);
}

/**
 * 校验查不到时index出参写成哨兵值, 而不是留着调用方的原值
 * @note 不写的话调用方分不清"没找到"和"找到下标0"
 */
TEST_CASE("find reports not found by writing the sentinel into the index") {
    SeqList list = make_list(8, {10, 20, 30});

    size_t index = 99;
    CHECK_EQ(TOYLIB_OK, seq_list_find(&list, &index, 42));

    CHECK_EQ(index, TOYLIB_SEQLIST_ELEMENT_NOT_FOUND);
    seq_list_free(&list);
}

/**
 * 校验free拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("free rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_free(nullptr));
}

/**
 * 校验未初始化的顺序表被调用时统一返回NOT_INITIALIZED
 * @note 零初始化的SeqList, data_为空; 三个操作都应该明确拒绝, 而不是给出误导性的错误码
 */
TEST_CASE("operations on an uninitialized list report not initialized") {
    SeqList list{};

    SUBCASE("insert") {
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, seq_list_insert(&list, 0, 1));
    }
    SUBCASE("delete") {
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, seq_list_delete(&list, 0, nullptr));
    }
    SUBCASE("find") {
        size_t index = 99;
        CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, seq_list_find(&list, &index, 1));
    }
}

/**
 * 校验free释放过之后再调用返回NOT_INITIALIZED
 * @note 第一次释放后data_被置空, 第二次会被"未初始化"检查拦下
 */
TEST_CASE("free rejects a list that was already freed") {
    SeqList list = make_list(8, {1, 2, 3});

    CHECK_EQ(TOYLIB_OK, seq_list_free(&list));
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, seq_list_free(&list));

    CHECK(list.data_ == nullptr);
    CHECK_EQ(list.capacity_, 0);
}

/**
 * 校验get拒绝非法参数: 顺序表为空指针报BAD_ARG_1, 出参为空指针报BAD_ARG_3
 */
TEST_CASE("get rejects invalid arguments") {
    SUBCASE("null list pointer") {
        Element value = 0;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_get(nullptr, 0, &value));
    }
    SUBCASE("null out parameter") {
        SeqList list = make_list(8, {1, 2, 3});
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_get(&list, 0, nullptr));
        seq_list_free(&list);
    }
}

/**
 * 校验空表取任何下标都越界
 */
TEST_CASE("get on an empty list reports out of range") {
    SeqList list = make_list(4, {});

    Element value = 99;
    CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_get(&list, 0, &value));

    CHECK_EQ(value, 99);
    seq_list_free(&list);
}

/**
 * 校验取值下标的上界: 长度和SIZE_MAX都要拒绝
 * @note 取值下标的合法区间是[0, length), 右端是开区间
 */
TEST_CASE("get rejects an index at or past the end") {
    SeqList list = make_list(8, {1, 2, 3});

    SUBCASE("index equals length") {
        Element value = 99;
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_get(&list, 3, &value));
        CHECK_EQ(value, 99);
    }
    SUBCASE("index is SIZE_MAX") {
        Element value = 99;
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_get(&list, SIZE_MAX, &value));
        CHECK_EQ(value, 99);
    }

    // 失败后内容不变
    CHECK_SEQ(list, 1, 2, 3);
    seq_list_free(&list);
}

/**
 * 校验未初始化的顺序表被取值时明确返回NOT_INITIALIZED
 */
TEST_CASE("get on an uninitialized list reports not initialized") {
    SeqList list{};

    Element value = 99;
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, seq_list_get(&list, 0, &value));
}

/**
 * 校验set拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("set rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_set(nullptr, 0, 1));
}

/**
 * 校验空表改写任何下标都越界
 */
TEST_CASE("set on an empty list reports out of range") {
    SeqList list = make_list(4, {});

    CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_set(&list, 0, 1));

    CHECK_EQ(list.length_, 0);
    seq_list_free(&list);
}

/**
 * 校验改写下标的上界: 长度和SIZE_MAX都要拒绝
 * @note 改写下标的合法区间是[0, length), 右端是开区间
 */
TEST_CASE("set rejects an index at or past the end") {
    SeqList list = make_list(8, {1, 2, 3});

    SUBCASE("index equals length") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_set(&list, 3, 9));
    }
    SUBCASE("index is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_set(&list, SIZE_MAX, 9));
    }

    // 失败后内容不变
    CHECK_SEQ(list, 1, 2, 3);
    seq_list_free(&list);
}

/**
 * 校验未初始化的顺序表被改写时明确返回NOT_INITIALIZED
 */
TEST_CASE("set on an uninitialized list reports not initialized") {
    SeqList list{};

    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, seq_list_set(&list, 0, 1));
}

/**
 * 校验length和capacity拒绝非法参数
 * @note 出参是唯一的返回值载体, 为空就没地方写结果
 */
TEST_CASE("length and capacity reject invalid arguments") {
    SeqList list = make_list(8, {1, 2, 3});

    SUBCASE("length with a null list pointer") {
        size_t length = 99;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_length(nullptr, &length));
    }
    SUBCASE("length with a null out parameter") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_length(&list, nullptr));
    }
    SUBCASE("capacity with a null list pointer") {
        size_t capacity = 99;
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_capacity(nullptr, &capacity));
    }
    SUBCASE("capacity with a null out parameter") {
        CHECK_EQ(TOYLIB_ERR_BAD_ARG_NULL, seq_list_capacity(&list, nullptr));
    }

    seq_list_free(&list);
}

TEST_SUITE_END();
