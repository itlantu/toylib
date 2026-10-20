/*
 * DList测试的辅助函数
 *
 * 只服务于test/dlist目录下的用例, 不做跨模块复用
 */

#ifndef TOYLIB_TEST_DLIST_SUPPORT_H
#define TOYLIB_TEST_DLIST_SUPPORT_H

#include <cstdlib>
#include <vector>

#include "doctest/doctest.h"
#include "toylib/dlist.h"

namespace toylib_test {

using Value = TOYLIB_DLIST_VALUE_TYPE;

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
 * 建一条空链表
 * @return 初始化好的链表, 用完需要free_nodes
 * @note 内部不检查返回值, 只适合关心链表结构的用例
 */
inline DList make_dlist() {
    DList dlist{};
    dlist_init(&dlist);
    return dlist;
}

/**
 * 释放链表的所有节点, 包括头尾哨兵
 * @param dlist 目标链表
 * @note 临时兜底: dlist_free还没实现, 实现之后这个函数换掉
 */
inline void free_nodes(DList& dlist) {
    DListNode* node = dlist.head_;
    while (node != nullptr) {
        DListNode* next = node->next_;
        std::free(node);
        node = next;
    }

    dlist.head_ = nullptr;
    dlist.tail_ = nullptr;
    dlist.length_ = 0;
}

}  // namespace toylib_test

// 断言链表内容和期望一致, 例如 CHECK_DLIST(dlist, 1, 2, 3)
#define CHECK_DLIST(list, ...) CHECK_EQ(toylib_test::to_values(list), (std::vector<toylib_test::Value>{__VA_ARGS__}))

#endif  // TOYLIB_TEST_DLIST_SUPPORT_H
