#ifndef TOYLIB_STACK_H
#define TOYLIB_STACK_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stddef.h>

#include "toylib/utils/error.h"

#ifndef TOYLIB_STACK_VALUE_TYPE
#define TOYLIB_STACK_VALUE_TYPE int
#endif  // TOYLIB_STACK_VALUE_TYPE

#ifndef TOYLIB_STACK_MAX_CAPACITY
#define TOYLIB_STACK_MAX_CAPACITY 100000
#endif  // TOYLIB_STACK_MAX_CAPACITY

typedef struct {
    size_t capacity_;
    size_t length_;
    TOYLIB_STACK_VALUE_TYPE* data_;
} Stack;

/**
 * 按容量分配内存, 初始化空栈, length_置为0
 * @param stack 目标栈, 不为空指针
 * @param capacity 栈容量, 取值[1, TOYLIB_STACK_MAX_CAPACITY]
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL,
 *         容量为0或超过上限返回TOYLIB_ERR_BAD_ARG_VALUE,
 *         内存不足返回TOYLIB_ERR_NOMEM
 * @note 内存由本函数分配, 用完需要stack_free释放
 */
ToylibError stack_init(Stack* stack, size_t capacity);

/**
 * 把值压入栈顶
 * @param stack 目标栈, 不为空指针
 * @param value 入栈的值
 * @return 成功或失败原因,
 *         栈未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         元素个数已达容量返回TOYLIB_ERR_CAPACITY_FULL
 * @note 时间复杂度O(1), 容量由stack_init决定, 压栈过程中不会变化
 */
ToylibError stack_push(Stack* stack, const TOYLIB_STACK_VALUE_TYPE value);

/**
 * 弹出栈顶元素
 * @param stack 目标栈, 不为空指针
 * @param value 被弹出的值, 为空指针则不赋值
 * @return 成功或失败原因,
 *         栈未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         空栈返回TOYLIB_ERR_EMPTY
 * @note 只减少元素个数, 不释放内存
 */
ToylibError stack_pop(Stack* stack, TOYLIB_STACK_VALUE_TYPE* value);

/**
 * 读取栈顶元素, 不弹出
 * @param stack 目标栈, 不为空指针
 * @param value 栈顶的值, 不为空指针
 * @return 成功或失败原因,
 *         栈未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         空栈返回TOYLIB_ERR_EMPTY
 * @note 只读操作, 调用之后栈内元素个数不变
 */
ToylibError stack_top(const Stack* stack, TOYLIB_STACK_VALUE_TYPE* value);

/**
 * 释放栈分配的内存, 并把字段复位(容量0, 长度0, data_为空)
 * @param stack 目标栈, 不为空指针
 * @return 成功或失败原因,
 *         栈未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 * @note 仅适用于stack_init创建的栈, 释放后需要重新init才能再次使用
 */
ToylibError stack_free(Stack* stack);

/**
 * 获取栈内元素个数
 * @param stack 目标栈, 不为空指针
 * @param length 栈内元素个数, 不为空指针
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL
 * @note 只读字段, 不检查栈是否已初始化, 未初始化时返回的是字段原值
 */
ToylibError stack_length(const Stack* stack, size_t* length);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_STACK_H
