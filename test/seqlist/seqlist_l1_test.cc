/*
 * L1契约级, 锁定每个API的最小可用路径和后置状态
 */

#include "doctest/doctest.h"
#include "seqlist_support.h"

using toylib_test::make_list;

TEST_SUITE_BEGIN("[SeqList][L1]");

/**
 * 校验create在正常容量下能分配内存
 * @note 契约: capacity_等于请求值, length_为 0, data_非空
 */
TEST_CASE("create allocates memory and leaves the list empty") {
    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_create(&list, 16));

    CHECK_EQ(list.capacity_, 16);
    CHECK_EQ(list.length_, 0);
    CHECK(list.data_ != nullptr);

    seq_list_free(&list);
}

/**
 * 校验create接受合法的最小容量 1
 * @note 容量是闭区间[1, MAX_CAPACITY], 只有0才算非法
 */
TEST_CASE("create accepts the minimal capacity of 1") {
    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_create(&list, 1));

    CHECK_EQ(list.capacity_, 1);
    CHECK_EQ(list.length_, 0);

    seq_list_free(&list);
}

/**
 * 校验init直接借用调用方的数组, 既不拷贝也不改写
 * @note 契约: data_指向传入数组, capacity_等于数组长度, length_为 0
 */
TEST_CASE("init borrows the caller array and leaves the list empty") {
    int buffer[8] = {10, 20, 30, 40, 50, 60, 70, 80};

    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_init(&list, buffer, 8));

    CHECK(list.data_ == buffer);
    CHECK_EQ(list.capacity_, 8);
    CHECK_EQ(list.length_, 0);
    // 借用不等于接管, 调用方的数据不该被动过
    CHECK_EQ(buffer[0], 10);
    CHECK_EQ(buffer[7], 80);
}

/**
 * 校验free释放内存后把三个字段都清零
 * @note 字段复位之后重复调用才是安全的, 见 L2 的 free can be called twice
 */
TEST_CASE("free releases memory and resets the fields") {
    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_create(&list, 8));

    CHECK_EQ(TOYLIB_OK, seq_list_free(&list));

    CHECK(list.data_ == nullptr);
    CHECK_EQ(list.length_, 0);
    CHECK_EQ(list.capacity_, 0);
}

/**
 * 校验往空表的下标0插入元素
 * @note 契约: 长度为1, 元素落在首位
 */
TEST_CASE("insert at index 0 into an empty list") {
    SeqList list = make_list(4, {});

    CHECK_EQ(TOYLIB_OK, seq_list_insert(&list, 0, 42));

    CHECK_SEQ(list, 42);
    seq_list_free(&list);
}

/**
 * 校验下标等于长度时是合法的插入位置, 效果是追加到尾部
 * @note 这是插入位置的上界, 再大就应该报OUT_OF_RANGE
 */
TEST_CASE("insert at index equal to length appends to the tail") {
    SeqList list = make_list(4, {1});

    CHECK_EQ(TOYLIB_OK, seq_list_insert(&list, 1, 2));

    CHECK_SEQ(list, 1, 2);
    seq_list_free(&list);
}

/**
 * 校验在中间插入时原有元素整体后移, 相对顺序不变
 */
TEST_CASE("insert in the middle shifts the tail right") {
    SeqList list = make_list(8, {1, 3, 4});

    CHECK_EQ(TOYLIB_OK, seq_list_insert(&list, 1, 2));

    CHECK_SEQ(list, 1, 2, 3, 4);
    seq_list_free(&list);
}

/**
 * 校验 delete 通过出参带回被删元素, 并把后面的元素整体前移
 * @note 契约: removed等于原来的data_[index], 长度减1
 */
TEST_CASE("delete returns the removed element and shifts the tail left") {
    SeqList list = make_list(16, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    int removed = 0;
    CHECK_EQ(TOYLIB_OK, seq_list_delete(&list, 5, &removed));

    CHECK_EQ(removed, 6);
    CHECK_SEQ(list, 1, 2, 3, 4, 5, 7, 8, 9, 10);
    seq_list_free(&list);
}

/**
 * 校验被删元素的出参可以是空指针, 此时只删除不取值
 */
TEST_CASE("delete accepts a null out parameter") {
    SeqList list = make_list(8, {1, 2, 3});

    CHECK_EQ(TOYLIB_OK, seq_list_delete(&list, 0, nullptr));

    CHECK_SEQ(list, 2, 3);
    seq_list_free(&list);
}

TEST_SUITE_END();
