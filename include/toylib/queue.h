#ifndef TOYLIB_QUEUE_H
#define TOYLIB_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stddef.h>

#include "toylib/utils/error.h"

#ifndef TOYLIB_QUEUE_VALUE_TYPE
#define TOYLIB_QUEUE_VALUE_TYPE int
#endif  // TOYLIB_QUEUE_VALUE_TYPE

#ifndef TOYLIB_QUEUE_MAX_SIZE
#define TOYLIB_QUEUE_MAX_SIZE 100000
#endif  // TOYLIB_QUEUE_MAX_SIZE

typedef struct {
    size_t capacity_;
    size_t front_;
    size_t length_;
    TOYLIB_QUEUE_VALUE_TYPE* data_;
} Queue;

/**
 * 按容量分配内存, 初始化空队列, front_和rear_都置为0
 * @param queue 目标队列, 不为空指针
 * @param capacity 队列容量, 取值[1, TOYLIB_QUEUE_MAX_SIZE]
 * @return 成功或失败原因,
 *         参数为空指针返回TOYLIB_ERR_BAD_ARG_NULL,
 *         容量为0或超过上限返回TOYLIB_ERR_BAD_ARG_VALUE,
 *         内存不足返回TOYLIB_ERR_NOMEM
 * @note 内存由本函数分配, 用完需要queue_free释放
 */
ToylibError queue_init(Queue* queue, size_t capacity);

/**
 * 把值放入队尾
 * @param queue 目标队列, 不为空指针
 * @param value 入队的值
 * @return 成功或失败原因,
 *         队列未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         元素个数已达容量返回TOYLIB_ERR_CAPACITY_FULL
 * @note 时间复杂度O(1), 下标回绕到数组头部时不需要搬移元素
 */
ToylibError queue_push(Queue* queue, const TOYLIB_QUEUE_VALUE_TYPE value);

/**
 * 弹出队头元素
 * @param queue 目标队列, 不为空指针
 * @param value 被弹出的值, 为空指针则不赋值
 * @return 成功或失败原因,
 *         队列未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         空队列返回TOYLIB_ERR_EMPTY
 * @note 只移动front_并减少元素个数, 不释放内存
 */
ToylibError queue_pop(Queue* queue, TOYLIB_QUEUE_VALUE_TYPE* value);

/**
 * 读取队头元素, 不弹出
 * @param queue 目标队列, 不为空指针
 * @param value 队头的值, 不为空指针
 * @return 成功或失败原因,
 *         队列未初始化返回TOYLIB_ERR_NOT_INITIALIZED,
 *         空队列返回TOYLIB_ERR_EMPTY
 * @note 只读操作, 调用之后队列内元素个数不变
 */
ToylibError queue_front(const Queue* queue, TOYLIB_QUEUE_VALUE_TYPE* value);

/**
 * 释放队列分配的内存, 并把字段复位(容量0, 元素个数0, front_和rear_归0, data_为空)
 * @param queue 目标队列, 不为空指针
 * @return 成功或失败原因,
 *         队列未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 * @note 仅适用于queue_init创建的队列, 释放后需要重新init才能再次使用
 */
ToylibError queue_free(Queue* queue);

/**
 * 获取队列内元素个数
 * @param queue 目标队列, 不为空指针
 * @param length 队列内元素个数, 不为空指针
 * @return 成功或失败原因,
 *         队列未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 */
ToylibError queue_length(const Queue* queue, size_t* length);

/**
 * 获取队列容量
 * @param queue 目标队列, 不为空指针
 * @param capacity 队列容量, 不为空指针
 * @return 成功或失败原因,
 *         队列未初始化返回TOYLIB_ERR_NOT_INITIALIZED
 */
ToylibError queue_capacity(const Queue* queue, size_t* capacity);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_QUEUE_H
