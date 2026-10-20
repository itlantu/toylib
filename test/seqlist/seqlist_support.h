/*
 * SeqList 测试的辅助函数
 *
 * 只服务于 test/seqlist 目录下的用例, 不做跨模块复用
 */

#ifndef TOYLIB_TEST_SEQLIST_SUPPORT_H
#define TOYLIB_TEST_SEQLIST_SUPPORT_H

#include <vector>

#include "doctest/doctest.h"
#include "toylib/seqlist.h"

namespace toylib_test {

using Element = TOYLIB_SEQLIST_ELEMENT_TYPE;

/**
 * 取出顺序表当前存放的元素
 * @param list 目标顺序表
 * @return 元素拷贝, 长度等于list的length_
 */
inline std::vector<Element> to_vector(const SeqList& list) {
    if (list.length_ == 0)
        return {};
    return std::vector<Element>(list.data_, list.data_ + list.length_);
}

/**
 * 建一个容量为capacity的顺序表, 并按顺序插入values
 * @param capacity 容量, 必须是合法值
 * @param values 初始元素, 个数不能超过容量
 * @return 建好的顺序表, 用完需要seq_list_free
 * @note 内部不检查返回值, 只适合容量和元素都合法的场景
 */
inline SeqList make_list(const size_t capacity, const std::initializer_list<Element> values) {
    SeqList list{};
    seq_list_create(&list, capacity);
    for (const Element value : values)
        seq_list_insert(&list, list.length_, value);
    return list;
}

/**
 * 断言create用这个容量会失败, 且不留下半初始化的状态
 * @param expected 期望的错误码
 * @param capacity 非法的容量
 */
inline void check_create_rejected(const ToylibError expected, const size_t capacity) {
    SeqList list{};
    CHECK_EQ(expected, seq_list_create(&list, capacity));
    CHECK(list.data_ == nullptr);
    CHECK_EQ(list.capacity_, 0);
}

/**
 * 断言init用这组参数会失败, 且不写入顺序表的字段
 * @param expected 期望的错误码
 * @param list 目标顺序表, 允许为空指针
 * @param data 元素数组, 允许为空指针
 * @param capacity 容量
 */
inline void check_init_rejected(const ToylibError expected, SeqList* list, Element* data, const size_t capacity) {
    CHECK_EQ(expected, seq_list_init(list, data, capacity));
    if (list != nullptr)
        CHECK(list->data_ == nullptr);
}

}  // namespace toylib_test

// 断言顺序表内容和期望一致, 例如CHECK_SEQ(list, 1, 2, 3)
#define CHECK_SEQ(list, ...) CHECK_EQ(toylib_test::to_vector(list), (std::vector<toylib_test::Element>{__VA_ARGS__}))

#endif  // TOYLIB_TEST_SEQLIST_SUPPORT_H
