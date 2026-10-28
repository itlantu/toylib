/*
 * DList测试的辅助函数
 *
 * 只服务于test/dlist目录下的用例, 不做跨模块复用
 */

#ifndef TOYLIB_TEST_DLIST_SUPPORT_H
#define TOYLIB_TEST_DLIST_SUPPORT_H

#include <initializer_list>
#include <vector>

#include "doctest/doctest.h"
#include "test_support.h"
#include "toylib/dlist.h"

namespace toylib_test {

using Value = TOYLIB_DLIST_VALUE_TYPE;

/**
 * L3随机序列测试的操作名, 下标就是用例里的op编号
 * @note 顺序必须和L3的switch保持一致; 长度顺便充当可抽取的操作数
 */
inline constexpr const char* kOpNames[] = {"insert", "delete", "find", "get", "set"};

/**
 * 从head_一路走到tail_, 收集真实节点的值
 * @param dlist 目标链表
 * @return 元素拷贝, 长度等于真实节点个数
 */
inline std::vector<Value> to_values(const DList& dlist) {
    std::vector<Value> values;
    for (const DListNode* node = dlist.head_->next_; node != dlist.tail_; node = node->next_) {
        values.push_back(node->value_);
    }
    return values;
}

/**
 * 从tail_倒着走回head_, 用来验证prev_链和next_链是否一致
 * @param dlist 目标链表
 * @return 倒序的元素拷贝
 */
inline std::vector<Value> to_values_reversed(const DList& dlist) {
    std::vector<Value> values;
    for (const DListNode* node = dlist.tail_->prev_; node != dlist.head_; node = node->prev_) {
        values.push_back(node->value_);
    }
    return values;
}

/**
 * 断言链表是空的: 长度为0, 且两个哨兵互指
 * @param dlist 目标链表
 * @note 删空之后如果哨兵没重新互指, 长度断言仍然是0, 必须连着结构一起查
 */
inline void check_empty(const DList& dlist) {
    CHECK_EQ(dlist.length_, 0);
    CHECK(dlist.head_->next_ == dlist.tail_);
    CHECK(dlist.tail_->prev_ == dlist.head_);
}

/**
 * 断言两个哨兵朝外的一侧是空的, 不会指到链表外面
 * @param dlist 目标链表
 */
inline void check_sentinels_outward(const DList& dlist) {
    CHECK(dlist.head_->prev_ == nullptr);
    CHECK(dlist.tail_->next_ == nullptr);
}

/**
 * 建一条空链表
 * @return 初始化好的链表, 用完需要dlist_free
 * @note 内部不检查返回值, 只适合关心链表结构的用例
 */
inline DList make_dlist() {
    DList dlist{};
    dlist_init(&dlist);
    return dlist;
}

/**
 * 建一条链表并按顺序填好初值
 * @param values 初始元素
 * @return 初始化好的链表, 用完需要dlist_free
 * @note 插入位置取length_, 所以链表里的顺序就是传入顺序
 */
inline DList make_dlist(const std::initializer_list<Value> values) {
    DList dlist = make_dlist();
    for (const Value value : values)
        dlist_insert(&dlist, dlist.length_, value);
    return dlist;
}

}  // namespace toylib_test

// 断言链表内容和期望一致, 例如 CHECK_DLIST(dlist, 1, 2, 3)
#define CHECK_DLIST(list, ...) CHECK_EQ(toylib_test::to_values(list), (std::vector<toylib_test::Value>{__VA_ARGS__}))

#endif  // TOYLIB_TEST_DLIST_SUPPORT_H
