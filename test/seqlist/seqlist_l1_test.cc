/*
 * L1契约级, 锁定每个API的最小可用路径和后置状态
 */

#include "doctest/doctest.h"
#include "seqlist_support.h"

using toylib_test::Element;
using toylib_test::make_list;

TEST_SUITE_BEGIN("[SeqList][L1]");

/**
 * 校验init在正常容量下能分配内存
 * @note 契约: capacity_等于请求值, length_为 0, data_非空
 */
TEST_CASE("init allocates memory and leaves the list empty") {
    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_init(&list, 16));

    CHECK_EQ(list.capacity_, 16);
    CHECK_EQ(list.length_, 0);
    CHECK(list.data_ != nullptr);

    seq_list_free(&list);
}

/**
 * 校验init接受合法的最小容量 1
 * @note 容量下界封闭: 1是合法的最小值, 0才算非法
 */
TEST_CASE("init accepts the minimal capacity of 1") {
    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_init(&list, 1));

    CHECK_EQ(list.capacity_, 1);
    CHECK_EQ(list.length_, 0);

    seq_list_free(&list);
}

/**
 * 校验free释放内存后把三个字段都清零
 * @note 字段复位之后才能被未初始化检查识别, 见L2的free rejects a list that was already freed
 */
TEST_CASE("free releases memory and resets the fields") {
    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_init(&list, 8));

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

/**
 * 校验find能定位到表中间的某个元素
 * @note 契约: 返回OK, index出参是元素在表中的下标
 */
TEST_CASE("find returns the index of an element") {
    SeqList list = make_list(8, {10, 20, 30, 40});
    size_t index = 99;

    CHECK_EQ(TOYLIB_OK, seq_list_find(&list, &index, 30));
    CHECK_EQ(index, 2);

    seq_list_free(&list);
}

/**
 * 校验find能定位到首个元素
 * @note 下标0是查找结果的下界, 不能和"没找到"混淆
 */
TEST_CASE("find locates the first element") {
    SeqList list = make_list(8, {10, 20, 30});
    size_t index = 99;

    CHECK_EQ(TOYLIB_OK, seq_list_find(&list, &index, 10));
    CHECK_EQ(index, 0);

    seq_list_free(&list);
}

/**
 * 校验find能定位到末尾元素
 * @note 下标length-1是查找结果的上界
 */
TEST_CASE("find locates the last element") {
    SeqList list = make_list(8, {10, 20, 30});
    size_t index = 99;

    CHECK_EQ(TOYLIB_OK, seq_list_find(&list, &index, 30));
    CHECK_EQ(index, 2);

    seq_list_free(&list);
}

/**
 * 校验元素重复出现时, find返回第一次出现的下标
 */
TEST_CASE("find returns the first occurrence of a duplicated element") {
    SeqList list = make_list(8, {5, 7, 7, 9});
    size_t index = 99;

    CHECK_EQ(TOYLIB_OK, seq_list_find(&list, &index, 7));
    CHECK_EQ(index, 1);

    seq_list_free(&list);
}

/**
 * 校验get能按下标取出值, 从第一个到最后一个
 * @note 契约: 返回OK, value出参是data_[index]; 读操作不能改动顺序表
 */
TEST_CASE("get returns the value at each index") {
    SeqList list = make_list(8, {10, 20, 30});

    Element value = 0;
    CHECK_EQ(TOYLIB_OK, seq_list_get(&list, 0, &value));
    CHECK_EQ(value, 10);
    CHECK_EQ(TOYLIB_OK, seq_list_get(&list, 1, &value));
    CHECK_EQ(value, 20);
    CHECK_EQ(TOYLIB_OK, seq_list_get(&list, 2, &value));
    CHECK_EQ(value, 30);

    // 连读三次之后顺序表必须原样不动
    CHECK_EQ(list.length_, 3);
    CHECK_SEQ(list, 10, 20, 30);
    seq_list_free(&list);
}

/**
 * 校验set能按下标改写值
 * @note 契约: 只改data_[index], 长度不变
 */
TEST_CASE("set replaces the value at each index") {
    SeqList list = make_list(8, {1, 2, 3});

    CHECK_EQ(TOYLIB_OK, seq_list_set(&list, 0, 10));
    CHECK_EQ(TOYLIB_OK, seq_list_set(&list, 1, 20));
    CHECK_EQ(TOYLIB_OK, seq_list_set(&list, 2, 30));

    CHECK_EQ(list.length_, 3);
    CHECK_SEQ(list, 10, 20, 30);
    seq_list_free(&list);
}

/**
 * 校验length返回元素个数, 并且跟着插入删除走
 */
TEST_CASE("length reports the element count") {
    SeqList list = make_list(8, {});

    size_t length = 99;
    CHECK_EQ(TOYLIB_OK, seq_list_length(&list, &length));
    CHECK_EQ(length, 0);

    CHECK_EQ(TOYLIB_OK, seq_list_insert(&list, 0, 1));
    CHECK_EQ(TOYLIB_OK, seq_list_insert(&list, 1, 2));
    CHECK_EQ(TOYLIB_OK, seq_list_length(&list, &length));
    CHECK_EQ(length, 2);

    CHECK_EQ(TOYLIB_OK, seq_list_delete(&list, 0, nullptr));
    CHECK_EQ(TOYLIB_OK, seq_list_length(&list, &length));
    CHECK_EQ(length, 1);

    seq_list_free(&list);
}

/**
 * 校验capacity返回init时确定的容量
 * @note 容量只在init时确定, 插入删除不会改变它
 */
TEST_CASE("capacity reports the allocated capacity") {
    SeqList list = make_list(16, {1, 2, 3});

    size_t capacity = 99;
    CHECK_EQ(TOYLIB_OK, seq_list_capacity(&list, &capacity));
    CHECK_EQ(capacity, 16);

    // 插入之后容量不变
    CHECK_EQ(TOYLIB_OK, seq_list_insert(&list, 3, 4));
    CHECK_EQ(TOYLIB_OK, seq_list_capacity(&list, &capacity));
    CHECK_EQ(capacity, 16);

    seq_list_free(&list);
}

TEST_SUITE_END();
