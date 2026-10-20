/*
 * L2边界级, 覆盖参数校验与下标边界, 并验证操作失败后状态不变
 */

#include <cstdint>

#include "dlist_support.h"
#include "doctest/doctest.h"

using toylib_test::free_nodes;
using toylib_test::make_dlist;

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
 * @note 当前实现没有检测head_是否为空, 本用例预期崩溃
 */
TEST_CASE("insert on an uninitialized list reports not initialized") {
    DList dlist{};

    CHECK_EQ(TOYLIB_ERR_NOT_INITIALIZED, dlist_insert(&dlist, 0, 1));
}

TEST_SUITE_END();
