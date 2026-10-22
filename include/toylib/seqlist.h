/*
 * seqlist模块实现顺序表(sequential list)
 */

#ifndef TOYLIB_SEQLIST_H
#define TOYLIB_SEQLIST_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stddef.h>
#include <stdint.h>

#include "toylib/utils/error.h"

#ifndef TOYLIB_SEQLIST_ELEMENT_TYPE
#define TOYLIB_SEQLIST_ELEMENT_TYPE int
#endif  // TOYLIB_SEQLIST_ELEMENT_TYPE

#ifndef TOYLIB_SEQLIST_ELEMENT_IS_EQUAL
#define TOYLIB_SEQLIST_ELEMENT_IS_EQUAL(a, b) a == b
#endif  // TOYLIB_SEQLIST_ELEMENT_IS_EQUAL

#ifndef TOYLIB_SEQLIST_MAX_CAPACITY
#define TOYLIB_SEQLIST_MAX_CAPACITY 100000
#endif  // TOYLIB_SEQLIST_MAX_CAPACITY

#ifndef TOYLIB_SEQLIST_ELEMENT_NOT_FOUND
#define TOYLIB_SEQLIST_ELEMENT_NOT_FOUND SIZE_MAX
#endif  // TOYLIB_SEQLIST_ELEMENT_NOT_FOUND

typedef struct {
    size_t capacity_;
    size_t length_;
    TOYLIB_SEQLIST_ELEMENT_TYPE* data_;
} SeqList;

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
ToylibError seq_list_insert(SeqList* seq_list, size_t index, const TOYLIB_SEQLIST_ELEMENT_TYPE element);

/**
 * 在顺序表中删除指定位置的元素
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 删除下标, 即被删除元素在顺序表的下标
 * @param deleted_element 被删除的元素, 为空指针则不赋值
 * @return 成功或失败原因
 */
ToylibError seq_list_delete(SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE* deleted_element);

/**
 * 在顺序表中查找指定元素的位置
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 元素下标, 如果顺序表找不到对应的元素则赋值为TOYLIB_SEQLIST_ELEMENT_NOT_FOUND
 * @param element 查找元素
 * @return 成功或失败原因
 */
ToylibError seq_list_find(const SeqList* seq_list, size_t* index, TOYLIB_SEQLIST_ELEMENT_TYPE element);

/**
 * 释放顺序表分配的内存, 并把字段复位(容量0, 长度0, data_为空)
 * 仅适用于seq_list_create创建的顺序表; seq_list_init只借用调用方内存, 不应调用本函数
 * @param seq_list 目标顺序表, 不为空指针
 * @return 成功或失败原因
 */
ToylibError seq_list_free(SeqList* seq_list);

/**
 * 获取顺序表中指定下标的值
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 值下标
 * @param value 顺序表中对应下标的元素, 不为空指针
 * @return 成功或失败原因
 */
ToylibError seq_list_get(const SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE* value);

/**
 * 修改顺序表中指定下标的值
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 值下标
 * @param value 修改元素, 会覆盖原有的值
 * @return 成功或失败原因
 */
ToylibError seq_list_set(SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE value);

/**
 * 获取顺序表长度
 * @param seq_list 目标顺序表, 不为空指针
 * @param length 目标顺序表的长度, 不为空指针
 * @return 成功或失败原因
 */
ToylibError seq_list_length(const SeqList* seq_list, size_t* length);

/**
 * 获取顺序表的容量
 * @param seq_list 目标顺序表, 不为空指针
 * @param capacity 目标顺序表的容量, 不为空指针
 * @return 成功或失败原因
 */
ToylibError seq_list_capacity(const SeqList* seq_list, size_t* capacity);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_SEQLIST_H
