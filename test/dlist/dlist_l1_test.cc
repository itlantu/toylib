/*
 * L1契约级, 锁定每个API的最小可用路径和后置状态
 */

#include "dlist_support.h"
#include "doctest/doctest.h"

using toylib_test::free_nodes;
using toylib_test::make_dlist;
using toylib_test::to_values_reversed;
using toylib_test::Value;

TEST_SUITE_BEGIN("[DList][L1]");

/**
 * 校验init能建出一条空链表
 * @note 契约: 头尾哨兵都存在且互指, 长度为0
 */
TEST_CASE("init creates an empty list") {
    DList dlist{};

    CHECK_EQ(TOYLIB_OK, dlist_init(&dlist));

    CHECK(dlist.head_ != nullptr);
    CHECK(dlist.tail_ != nullptr);
    CHECK_EQ(dlist.length_, 0);
    CHECK(dlist.head_->next_ == dlist.tail_);
    CHECK(dlist.tail_->prev_ == dlist.head_);

    free_nodes(dlist);
}

/**
 * 校验两个哨兵朝外的一侧是空的, 不会指到链表外面
 */
TEST_CASE("init leaves the sentinels unlinked outward") {
    DList dlist = make_dlist();

    CHECK(dlist.head_->prev_ == nullptr);
    CHECK(dlist.tail_->next_ == nullptr);

    free_nodes(dlist);
}

/**
 * 校验往空链表的下标0插入
 * @note 契约: 长度为1, 值落在头哨兵之后
 */
TEST_CASE("insert at index 0 into an empty list") {
    DList dlist = make_dlist();

    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 0, 42));

    CHECK_EQ(dlist.length_, 1);
    CHECK_DLIST(dlist, 42);
    free_nodes(dlist);
}

/**
 * 校验下标等于长度时是合法的插入位置, 效果是追加到尾部
 * @note 这是插入位置的上界, 再大就应该报OUT_OF_RANGE
 */
TEST_CASE("insert at index equal to length appends") {
    DList dlist = make_dlist();

    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 0, 1));
    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 1, 2));

    CHECK_EQ(dlist.length_, 2);
    CHECK_DLIST(dlist, 1, 2);
    free_nodes(dlist);
}

/**
 * 校验在中间插入时相对顺序不变
 */
TEST_CASE("insert in the middle keeps the order") {
    DList dlist = make_dlist();

    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 0, 1));
    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 1, 3));
    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 1, 2));

    CHECK_EQ(dlist.length_, 3);
    CHECK_DLIST(dlist, 1, 2, 3);
    free_nodes(dlist);
}

/**
 * 校验插入之后prev_链和next_链指向同一串节点
 * @note 双链表的典型错误是只接了next_忘了接prev_, 只测正向顺序发现不了
 */
TEST_CASE("insert keeps both directions consistent") {
    DList dlist = make_dlist();

    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 0, 1));
    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 1, 2));
    CHECK_EQ(TOYLIB_OK, dlist_insert(&dlist, 2, 3));

    CHECK_EQ(to_values_reversed(dlist), (std::vector<Value>{3, 2, 1}));
    free_nodes(dlist);
}

TEST_SUITE_END();
