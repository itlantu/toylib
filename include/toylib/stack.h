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
 * 按容量分配内存, 初始化栈
 * @param stack 目标栈, 不为空指针
 * @param capacity 栈容量, 取值[1, TOYLIB_STACK_MAX_CAPACITY]
 * @return 成功或失败原因
 * @note 内存由本函数分配, 用完需要stack_free释放;
 */
ToylibError stack_init(Stack* stack, size_t capacity);

/**
 * 把值压入栈顶
 * @param stack 目标栈, 不为空指针
 * @param value 入栈的值
 * @return 成功或失败原因, 容量已满时返回TOYLIB_ERR_CAPACITY_FULL
 */
ToylibError stack_push(Stack* stack, const TOYLIB_STACK_VALUE_TYPE value);

/**
 * 弹出栈顶元素
 * @param stack 目标栈, 不为空指针
 * @param value 被弹出的值, 为空指针则不赋值
 * @return 成功或失败原因, 空栈时返回TOYLIB_ERR_EMPTY
 */
ToylibError stack_pop(Stack* stack, TOYLIB_STACK_VALUE_TYPE* value);

/**
 * 读取栈顶元素, 不弹出
 * @param stack 目标栈, 不为空指针
 * @param value 栈顶的值, 不为空指针
 * @return 成功或失败原因, 空栈时返回TOYLIB_ERR_EMPTY
 * @note 只读操作, 调用之后栈内元素个数不变
 */
ToylibError stack_top(const Stack* stack, TOYLIB_STACK_VALUE_TYPE* value);

/**
 * 释放栈分配的内存, 并把字段复位(容量0, 长度0, data_为空)
 * @param stack 目标栈, 不为空指针
 * @return 成功或失败原因
 */
ToylibError stack_free(Stack* stack);

/**
 * 获取栈内元素个数
 * @param stack 目标栈, 不为空指针
 * @param length 栈内元素个数, 不为空指针
 * @return 成功或失败原因
 */
ToylibError stack_length(const Stack* stack, size_t* length);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_STACK_H
