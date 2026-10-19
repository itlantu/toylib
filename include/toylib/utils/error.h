#ifndef TOYLIB_ERROR_H
#define TOYLIB_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum {
    // OK, 无异常
    TOYLIB_OK = 0,

    // NOMEM, 内存分配失败
    TOYLIB_ERR_NOMEM,
    // OUT_OF_RANGE, 下标越界
    TOYLIB_ERR_OUT_OF_RANGE,
    // CAPACITY_FULL, 当前数据集结构的容量已满, 无法插入元素
    TOYLIB_ERR_CAPACITY_FULL,
    // ERR_EMPTY, 当前数据结构的容量为空, 无法删除元素
    TOYLIB_ERR_EMPTY,

    // BAD_ARG_1, 函数调用时, 第一个参数不合法
    TOYLIB_ERR_BAD_ARG_1,
    // BAD_ARG_2, 函数调用时, 第二个参数不合法
    TOYLIB_ERR_BAD_ARG_2,
    // BAD_ARG_3, 函数调用时, 第三个参数不合法
    TOYLIB_ERR_BAD_ARG_3,
    // BAD_ARG_4, 函数调用时, 第四个参数不合法
    TOYLIB_ERR_BAD_ARG_4,

}ToylibError;

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TOYLIB_ERROR_H
