#ifndef TOYLIB_ERROR_H
#define TOYLIB_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum {
    // OK, 无异常
    TOYLIB_OK = 0,

    // BAD_ARG_NULL, 参数是空指针
    TOYLIB_ERR_BAD_ARG_NULL,
    // BAD_ARG_VALUE, 参数值非法(容量为0或者大于上限)
    TOYLIB_ERR_BAD_ARG_VALUE,
    // NOT_INITIALIZED, 未初始化
    TOYLIB_ERR_NOT_INITIALIZED,

    // OUT_OF_RANGE, 下标越界
    TOYLIB_ERR_OUT_OF_RANGE,
    // CAPACITY_FULL, 当前数据集结构的容量已满, 无法插入元素
    TOYLIB_ERR_CAPACITY_FULL,
    // ERR_EMPTY, 当前数据结构的容量为空, 无法删除元素
    TOYLIB_ERR_EMPTY,

    // NOMEM, 内存分配失败
    TOYLIB_ERR_NOMEM,

    // UNKNOWN, 未知错误, 没有被ToylibError定义
    TOYLIB_ERR_UNKNOWN
} ToylibError;

/**
 * 获取错误码的名称(枚举名)
 * @param error 错误码
 * @return 错误码的枚举名
 * @note 错误码落在已定义的区间之外时返回TOYLIB_ERR_UNKNOWN对应的字符串
 */
const char* toylib_error_name(ToylibError error);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_ERROR_H
