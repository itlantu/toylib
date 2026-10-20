/*
 * L2边界级, 覆盖参数校验与下标边界, 并验证操作失败后状态不变
 */

#include <cstdint>

#include "dlist_support.h"
#include "doctest/doctest.h"

using toylib_test::free_nodes;
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
    free_nodes(dlist);
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
    free_nodes(dlist);
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
    free_nodes(dlist);
}

TEST_SUITE_END();
