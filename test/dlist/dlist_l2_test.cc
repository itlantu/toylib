/*
 * L2边界级, 覆盖参数校验与下标边界, 并验证操作失败后状态不变
 */

#include <cstdint>

#include "dlist_support.h"
#include "doctest/doctest.h"

using toylib_test::make_dlist;
using toylib_test::Value;

TEST_SUITE_BEGIN("[DList][L2]");

/**
 * 校验init拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("init rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_init(nullptr));
}

/**
 * 校验insert拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("insert rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_insert(nullptr, 0, 1));
}

/**
 * 校验插入位置的上界: 长度加一和SIZE_MAX都要拒绝
 * @note 插入位置的合法区间是[0, length]
 */
TEST_CASE("insert rejects an index past the end") {
    DList dlist = make_dlist();

    SUBCASE("index is length plus one") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_insert(&dlist, 1, 9));
    }
    SUBCASE("index is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_insert(&dlist, SIZE_MAX, 9));
    }

    // 失败后链表结构不变
    CHECK_EQ(dlist.length_, 0);
    CHECK(dlist.head_->next_ == dlist.tail_);
    CHECK(dlist.tail_->prev_ == dlist.head_);
    dlist_free(&dlist);
}

/**
 * 校验未初始化的链表被调用时明确返回NOT_INITIALIZED
 */
TEST_CASE("insert on an uninitialized list reports not initialized") {
    DList dlist{};

    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_insert(&dlist, 0, 1));
}

/**
 * 校验delete拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("delete rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_delete(nullptr, 0, nullptr));
}

/**
 * 校验未初始化的链表被删除时明确返回NOT_INITIALIZED
 */
TEST_CASE("delete on an uninitialized list reports not initialized") {
    DList dlist{};

    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_delete(&dlist, 0, nullptr));
}

/**
 * 校验空链表删除返回EMPTY
 * @note 与seq_list_delete保持一致: 表里没有元素和下标越界是两种不同的错误
 */
TEST_CASE("delete on an empty list reports empty") {
    DList dlist = make_dlist();

    Value removed = 0;
    CHECK_EQ(TOYLIB_ERR_EMPTY, dlist_delete(&dlist, 0, &removed));

    CHECK_EQ(dlist.length_, 0);
    dlist_free(&dlist);
}

/**
 * 校验删除位置的上界: 长度和SIZE_MAX都要拒绝
 * @note 删除位置的合法区间是[0, length), 右端是开区间
 */
TEST_CASE("delete rejects an index at or past the end") {
    DList dlist = make_dlist({1, 2, 3});

    SUBCASE("index equals length") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_delete(&dlist, 3, nullptr));
    }
    SUBCASE("index is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_delete(&dlist, SIZE_MAX, nullptr));
    }

    // 失败后链表结构和内容都不变
    CHECK_EQ(dlist.length_, 3);
    CHECK_DLIST(dlist, 1, 2, 3);
    dlist_free(&dlist);
}

/**
 * 校验find拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("find rejects a null list pointer") {
    size_t index = 99;
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_find(nullptr, &index, 1));
}

/**
 * 校验find拒绝空的出参, 返回BAD_ARG_2
 * @note 没地方写下标就没法带回结果, 属于调用方的错误
 */
TEST_CASE("find rejects a null index out parameter") {
    DList dlist = make_dlist({1, 2, 3});

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_2, dlist_find(&dlist, nullptr, 2));
    dlist_free(&dlist);
}

/**
 * 校验空链表里查不到任何值
 * @note 查不到时index出参写成哨兵值TOYLIB_DLIST_VALUE_NOT_FOUND
 */
TEST_CASE("find on an empty list reports not found") {
    DList dlist = make_dlist();

    size_t index = 99;
    CHECK_EQ(TOYLIB_OK, dlist_find(&dlist, &index, 1));

    CHECK_EQ(index, TOYLIB_DLIST_VALUE_NOT_FOUND);
    dlist_free(&dlist);
}

/**
 * 校验查不到时index出参写成哨兵值, 而不是留着调用方的原值
 * @note 不写的话调用方分不清"没找到"和"找到下标0"
 */
TEST_CASE("find reports not found by writing the sentinel") {
    DList dlist = make_dlist({10, 20, 30});

    size_t index = 99;
    CHECK_EQ(TOYLIB_OK, dlist_find(&dlist, &index, 42));

    CHECK_EQ(index, TOYLIB_DLIST_VALUE_NOT_FOUND);
    dlist_free(&dlist);
}

/**
 * 校验未初始化的链表被查找时明确返回NOT_INITIALIZED
 */
TEST_CASE("find on an uninitialized list reports not initialized") {
    DList dlist{};

    size_t index = 99;
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_find(&dlist, &index, 1));
}

/**
 * 校验free拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("free rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_free(nullptr));
}

/**
 * 校验未初始化的链表被释放时明确返回NOT_INITIALIZED
 */
TEST_CASE("free on an uninitialized list reports not initialized") {
    DList dlist{};

    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_free(&dlist));
}

/**
 * 校验释放过之后再调用返回NOT_INITIALIZED
 * @note 与seq_list_free一致: 第一次释放后字段要复位, 第二次才能被拦住
 */
TEST_CASE("free can be called twice") {
    DList dlist = make_dlist({1, 2, 3});

    CHECK_EQ(TOYLIB_OK, dlist_free(&dlist));
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_free(&dlist));
}

/**
 * 校验get拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("get rejects a null list pointer") {
    Value value = 0;
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_get(nullptr, 0, &value));
}

/**
 * 校验get拒绝空的出参, 返回BAD_ARG_3
 * @note value是第三个参数, 所以是BAD_ARG_3; 没地方写结果就没法返回
 */
TEST_CASE("get rejects a null out parameter") {
    DList dlist = make_dlist({1, 2, 3});

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_3, dlist_get(&dlist, 0, nullptr));
    dlist_free(&dlist);
}

/**
 * 校验空链表取任何下标都越界
 */
TEST_CASE("get on an empty list reports out of range") {
    DList dlist = make_dlist();

    Value value = 99;
    CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_get(&dlist, 0, &value));

    CHECK_EQ(value, 99);
    dlist_free(&dlist);
}

/**
 * 校验取值下标的上界: 长度和SIZE_MAX都要拒绝
 * @note 取值下标的合法区间是[0, length), 右端是开区间
 */
TEST_CASE("get rejects an index at or past the end") {
    DList dlist = make_dlist({1, 2, 3});

    SUBCASE("index equals length") {
        Value value = 99;
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_get(&dlist, 3, &value));
        CHECK_EQ(value, 99);
    }
    SUBCASE("index is SIZE_MAX") {
        Value value = 99;
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_get(&dlist, SIZE_MAX, &value));
        CHECK_EQ(value, 99);
    }

    // 失败后链表结构和内容都不变
    CHECK_EQ(dlist.length_, 3);
    CHECK_DLIST(dlist, 1, 2, 3);
    dlist_free(&dlist);
}

/**
 * 校验未初始化的链表被取值时明确返回NOT_INITIALIZED
 */
TEST_CASE("get on an uninitialized list reports not initialized") {
    DList dlist{};

    Value value = 99;
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_get(&dlist, 0, &value));
}

/**
 * 校验set拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("set rejects a null list pointer") {
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_set(nullptr, 0, 1));
}

/**
 * 校验未初始化的链表被改写时明确返回NOT_INITIALIZED
 */
TEST_CASE("set on an uninitialized list reports not initialized") {
    DList dlist{};

    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_set(&dlist, 0, 1));
}

/**
 * 校验空链表改写任何下标都越界
 */
TEST_CASE("set on an empty list reports out of range") {
    DList dlist = make_dlist();

    CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_set(&dlist, 0, 1));
    CHECK_EQ(dlist.length_, 0);
    dlist_free(&dlist);
}

/**
 * 校验改写下标的上界: 长度和SIZE_MAX都要拒绝
 * @note 改写下标的合法区间是[0, length), 右端是开区间
 */
TEST_CASE("set rejects an index at or past the end") {
    DList dlist = make_dlist({1, 2, 3});

    SUBCASE("index equals length") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_set(&dlist, 3, 9));
    }
    SUBCASE("index is SIZE_MAX") {
        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, dlist_set(&dlist, SIZE_MAX, 9));
    }

    // 失败后链表结构和内容都不变
    CHECK_EQ(dlist.length_, 3);
    CHECK_DLIST(dlist, 1, 2, 3);
    dlist_free(&dlist);
}

/**
 * 校验length拒绝空指针, 返回BAD_ARG_1
 */
TEST_CASE("length rejects a null list pointer") {
    size_t length = 99;
    CHECK_EQ(TOYLIB_ERR_BAD_ARG_1, dlist_length(nullptr, &length));
}

/**
 * 校验length拒绝空的出参, 返回BAD_ARG_2
 * @note length是第二个参数, 没地方写结果就没法返回
 */
TEST_CASE("length rejects a null out parameter") {
    DList dlist = make_dlist({1, 2, 3});

    CHECK_EQ(TOYLIB_ERR_BAD_ARG_2, dlist_length(&dlist, nullptr));
    dlist_free(&dlist);
}

/**
 * 校验未初始化的链表被取长度时明确返回NOT_INITIALIZED
 */
TEST_CASE("length on an uninitialized list reports not initialized") {
    DList dlist{};

    size_t length = 99;
    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_length(&dlist, &length));
}

TEST_SUITE_END();
