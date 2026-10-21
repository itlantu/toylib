#ifndef TOYLIB_DLIST_H
#define TOYLIB_DLIST_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stddef.h>
#include <stdint.h>

#include "toylib/utils/error.h"

#ifndef TOYLIB_DLIST_VALUE_TYPE
#define TOYLIB_DLIST_VALUE_TYPE int
#endif  // TOYLIB_DLIST_VALUE_TYPE

#ifndef TOYLIB_DLIST_VALUE_NOT_FOUND
#define TOYLIB_DLIST_VALUE_NOT_FOUND SIZE_MAX
#endif  // TOYLIB_DLIST_VALUE_NOT_FOUND

typedef struct DListNode {
    struct DListNode* prev_;
    struct DListNode* next_;
    TOYLIB_DLIST_VALUE_TYPE value_;
} DListNode;

typedef struct {
    DListNode* head_;
    DListNode* tail_;
    size_t length_;
} DList;

/**
 * 初始化链表
 * @param dlist 目标链表, 不为空指针
 * @return 成功或失败原因
 */
ToylibError dlist_init(DList* dlist);

/**
 * 在链表的指定位置插入值
 * @param dlist 目标链表, 不为空指针
 * @param index 插入下标, 即插入值在链表的下标
 * @param value 插入值
 * @return 成功或失败原因
 */
ToylibError dlist_insert(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE value);

/**
 * 删除链表指定位置的节点
 * @param dlist 目标链表, 不为空指针
 * @param index 删除值的下标, 即被删除节点在链表的下标
 * @param deleted_value 被删除节点的值, 为空指针则不赋值
 * @return 成功或失败原因
 */
ToylibError dlist_delete(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE* deleted_value);

/**
 * 在链表中查找指定值的位置
 * @param dlist 目标链表, 不为空指针
 * @param index 值下标, 如果顺序表找不到对应的元素则赋值为TOYLIB_DLIST_VALUE_NOT_FOUND
 * @param value 查找值
 * @return 成功或失败原因
 */
ToylibError dlist_find(const DList* dlist, size_t* index, TOYLIB_DLIST_VALUE_TYPE value);

/**
 * 释放链表分配的内存, 包括头指针(head_)和尾指针(tail_), 两指针置为空指针, 同时length_置为0
 * @param dlist 目标链表, 不为空指针
 * @return 成功或失败原因
 */
ToylibError dlist_free(DList* dlist);

/**
 * 获取链表中指定下标的值
 * @param dlist 目标链表, 不为空指针
 * @param index 值下标
 * @param value 链表中对应下标的值, 不为空指针
 * @return 成功或失败原因
 */
ToylibError dlist_get(const DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE* value);

/**
 * 修改链表中指定下标的值
 * @param dlist 目标链表, 不为空指针
 * @param index 值下标
 * @param value 修改值, 会覆盖原有的值
 * @return 成功或失败原因
 */
ToylibError dlist_set(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE value);

/**
 * 获取链表长度
 * @param dlist 目标链表, 不为空指针
 * @param length 目标链表的长度, 不为空指针
 * @return 成功或失败原因
 */
ToylibError dlist_length(const DList* dlist, size_t* length);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_DLIST_H
