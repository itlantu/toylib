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
 * 按容量分配内存, 初始化空顺序表, length_置为0
 * @param seq_list 目标顺序表, 不为空指针
 * @param capacity 顺序表容量, 即能存放的元素个数, 取值[1, TOYLIB_SEQLIST_MAX_CAPACITY]
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL,
 *         容量为0或超过上限返回TOYLIB_ERR_BAD_ARG_VALUE,
 *         内存不足返回TOYLIB_ERR_NOMEM
 * @note 内存由本函数分配, 用完需要seq_list_free释放
 */
ToylibError seq_list_init(SeqList* seq_list, size_t capacity);

/**
 * 在顺序表的指定位置插入元素, 插入点之后的元素整体后移
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 插入下标, 即插入元素在顺序表的下标, 取值[0, length]
 * @param element 插入的元素
 * @return 成功或失败原因,
 *         顺序表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         下标大于length返回TOYLIB_ERR_OUT_OF_RANGE,
 *         元素个数已达容量返回TOYLIB_ERR_CAPACITY_FULL
 * @note 先判下标再判容量, index等于length时是追加到末尾; 时间复杂度O(n)
 */
ToylibError seq_list_insert(SeqList* seq_list, size_t index, const TOYLIB_SEQLIST_ELEMENT_TYPE element);

/**
 * 在顺序表中删除指定位置的元素, 被删元素之后的元素整体前移
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 删除下标, 即被删除元素在顺序表的下标, 取值[0, length)
 * @param deleted_element 被删除的元素, 为空指针则不赋值
 * @return 成功或失败原因,
 *         顺序表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         空表返回TOYLIB_ERR_EMPTY,
 *         下标不小于length返回TOYLIB_ERR_OUT_OF_RANGE
 * @note 先判空表再判下标; 时间复杂度O(n)
 */
ToylibError seq_list_delete(SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE* deleted_element);

/**
 * 在顺序表中查找指定元素的位置
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 元素下标, 找不到对应的元素则赋值为TOYLIB_SEQLIST_ELEMENT_NOT_FOUND, 不为空指针
 * @param element 查找元素
 * @return 成功或失败原因,
 *         顺序表未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 * @note 元素重复出现时返回第一次出现的下标; 查不到也算成功, 靠index的哨兵值区分; 时间复杂度O(n)
 */
ToylibError seq_list_find(const SeqList* seq_list, size_t* index, TOYLIB_SEQLIST_ELEMENT_TYPE element);

/**
 * 释放顺序表分配的内存, 并把字段复位(容量0, 长度0, data_为空)
 * @param seq_list 目标顺序表, 不为空指针
 * @return 成功或失败原因,
 *         顺序表未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 * @note 只能释放seq_list_init分配的内存, 释放后再次调用返回TOYLIB_ERR_NOT_INITIALIZED
 */
ToylibError seq_list_free(SeqList* seq_list);

/**
 * 获取顺序表中指定下标的值
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 值下标, 取值[0, length)
 * @param value 顺序表中对应下标的元素, 不为空指针
 * @return 成功或失败原因,
 *         顺序表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         下标不小于length返回TOYLIB_ERR_OUT_OF_RANGE
 * @note 只读操作, 调用之后顺序表内容不变
 */
ToylibError seq_list_get(const SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE* value);

/**
 * 修改顺序表中指定下标的值
 * @param seq_list 目标顺序表, 不为空指针
 * @param index 值下标, 取值[0, length)
 * @param value 修改元素, 会覆盖原有的值
 * @return 成功或失败原因,
 *         顺序表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         下标不小于length返回TOYLIB_ERR_OUT_OF_RANGE
 */
ToylibError seq_list_set(SeqList* seq_list, size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE value);

/**
 * 获取顺序表长度
 * @param seq_list 目标顺序表, 不为空指针
 * @param length 目标顺序表的长度, 不为空指针
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL
 * @note 只读字段, 不检查顺序表是否已初始化, 未初始化时返回的是字段原值
 */
ToylibError seq_list_length(const SeqList* seq_list, size_t* length);

/**
 * 获取顺序表的容量
 * @param seq_list 目标顺序表, 不为空指针
 * @param capacity 目标顺序表的容量, 不为空指针
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL
 * @note 只读字段, 不检查顺序表是否已初始化, 未初始化时返回的是字段原值
 */
ToylibError seq_list_capacity(const SeqList* seq_list, size_t* capacity);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_SEQLIST_H
