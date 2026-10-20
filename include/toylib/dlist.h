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
ToylibError dlist_find(DList* dlist, size_t* index, TOYLIB_DLIST_VALUE_TYPE value);
ToylibError dlist_free(DList* dlist);

ToylibError dlist_get(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE* value);
ToylibError dlist_set(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE value);
ToylibError dlist_length(DList* dlist, size_t* length);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_DLIST_H
