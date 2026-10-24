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

#ifndef TOYLIB_DLIST_VALUE_IS_EQUAL
#define TOYLIB_DLIST_VALUE_IS_EQUAL(a, b) a == b
#endif  // TOYLIB_DLIST_VALUE_IS_EQUAL

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
 * 初始化链表, 创建头尾哨兵节点, length_置为0
 * @param dlist 目标链表, 不为空指针
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL,
 *         哨兵节点分配失败返回TOYLIB_ERR_NOMEM
 * @note 内存由本函数分配, 用完需要dlist_free释放
 */
ToylibError dlist_init(DList* dlist);

/**
 * 在链表的指定位置插入值
 * @param dlist 目标链表, 不为空指针
 * @param index 插入下标, 即插入值在链表的下标, 取值[0, length]
 * @param value 插入值
 * @return 成功或失败原因,
 *         链表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         下标大于length返回TOYLIB_ERR_OUT_OF_RANGE,
 *         节点分配失败返回TOYLIB_ERR_NOMEM
 * @note 从head_往后走到插入点, 时间复杂度O(n)
 */
ToylibError dlist_insert(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE value);

/**
 * 删除链表指定位置的节点
 * @param dlist 目标链表, 不为空指针
 * @param index 删除值的下标, 即被删除节点在链表的下标, 取值[0, length)
 * @param deleted_value 被删除节点的值, 为空指针则不赋值
 * @return 成功或失败原因,
 *         链表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         空链表返回TOYLIB_ERR_EMPTY,
 *         下标不小于length返回TOYLIB_ERR_OUT_OF_RANGE
 * @note 先判空链表再判下标; 被删节点占用的内存由本函数释放; 时间复杂度O(n)
 */
ToylibError dlist_delete(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE* deleted_value);

/**
 * 在链表中查找指定值的位置
 * @param dlist 目标链表, 不为空指针
 * @param index 值下标, 找不到对应的值则赋值为TOYLIB_DLIST_VALUE_NOT_FOUND, 不为空指针
 * @param value 查找值
 * @return 成功或失败原因,
 *         链表未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 * @note 值重复出现时返回第一次出现的下标; 查不到也算成功, 靠index的哨兵值区分; 时间复杂度O(n)
 */
ToylibError dlist_find(const DList* dlist, size_t* index, TOYLIB_DLIST_VALUE_TYPE value);

/**
 * 释放链表分配的内存, 包括全部节点和头尾哨兵, 并把字段复位(head_和tail_为空, length_为0)
 * @param dlist 目标链表, 不为空指针
 * @return 成功或失败原因,
 *         链表未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 * @note 释放后需要重新init才能再次使用
 */
ToylibError dlist_free(DList* dlist);

/**
 * 获取链表中指定下标的值
 * @param dlist 目标链表, 不为空指针
 * @param index 值下标, 取值[0, length)
 * @param value 链表中对应下标的值, 不为空指针
 * @return 成功或失败原因,
 *         链表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         下标不小于length返回TOYLIB_ERR_OUT_OF_RANGE
 * @note 只读操作, 调用之后链表内容不变
 */
ToylibError dlist_get(const DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE* value);

/**
 * 修改链表中指定下标的值
 * @param dlist 目标链表, 不为空指针
 * @param index 值下标, 取值[0, length)
 * @param value 修改值, 会覆盖原有的值
 * @return 成功或失败原因,
 *         链表未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         下标不小于length返回TOYLIB_ERR_OUT_OF_RANGE
 */
ToylibError dlist_set(DList* dlist, size_t index, TOYLIB_DLIST_VALUE_TYPE value);

/**
 * 获取链表长度
 * @param dlist 目标链表, 不为空指针
 * @param length 目标链表的长度, 不为空指针
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL,
 *         链表未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 */
ToylibError dlist_length(const DList* dlist, size_t* length);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_DLIST_H
