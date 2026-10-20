/*
 * seqlist模块实现顺序表(sequential list)
 */

#ifndef TOYLIB_SEQLIST_H
#define TOYLIB_SEQLIST_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stddef.h>
#include "toylib/utils/error.h"

#ifndef TOYLIB_SEQLIST_ELEMENT_TYPE
#define TOYLIB_SEQLIST_ELEMENT_TYPE int
#endif // TOYLIB_SEQLIST_ELEMENT_TYPE

#ifndef TOYLIB_SEQLIST_ELEMENT_IS_EQUAL
#define TOYLIB_SEQLIST_ELEMENT_IS_EQUAL(a, b) a == b
#endif // TOYLIB_SEQLIST_ELEMENT_IS_EQUAL

#ifndef TOYLIB_SEQLIST_MAX_CAPACITY
#define TOYLIB_SEQLIST_MAX_CAPACITY 100000
#endif  // TOYLIB_SEQLIST_MAX_CAPACITY

typedef struct {
    size_t capacity_;
    size_t length_;
    TOYLIB_SEQLIST_ELEMENT_TYPE* data_;
}SeqList;

/**
 * 创建顺序表, 根据容量动态分配内存
 * @param seq_list 目标顺序表, 不为空指针
 * @param capacity 顺序表容量, 即顺序表可以存放的元素个数
 * @return 成功或失败原因
 */
ToylibError seq_list_create(SeqList* seq_list, size_t capacity);

/**
 * 根据已分配的内存, 初始化顺序表
 * @param seq_list 目标顺序表, 不为空指针
 * @param data 元素类型为TOYLIB_SEQLIST_ELEMENT_TYPE的数组
 * @param capacity 顺序表容量, 即data的长度
 * @return 成功或失败原因
 */
ToylibError seq_list_init(SeqList* seq_list, TOYLIB_SEQLIST_ELEMENT_TYPE* data, size_t capacity);

/**
 * 在顺序表的指定位置插入元素
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 插入下标, 即插入元素在顺序表的下标
 * @param element 插入的元素
 * @return 成功或失败原因
 */
ToylibError seq_list_insert(SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE element);

/**
 * 在顺序表中删除指定位置的元素
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 删除下标, 即被删除元素在顺序表的下标
 * @param deleted_element 被删除的元素, 为空指针则不赋值
 * @return 成功或失败原因
 */
ToylibError seq_list_delete(SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE* deleted_element);

ToylibError seq_list_find(SeqList* seq_list, size_t* index, ToylibError element);

/**
 * 释放顺序表分配的内存
 * @param seq_list 目标顺序表, 不为空指针
 * @return 成功或失败原因
 */
ToylibError seq_list_free(SeqList* seq_list);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TOYLIB_SEQLIST_H
