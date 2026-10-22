#include "toylib/utils/error.h"

const char* toylib_error_name(const ToylibError error) {
    static const char* names[] = {
      "TOYLIB_OK",
      "TOYLIB_ERR_BAD_ARG_NULL",
      "TOYLIB_ERR_BAD_ARG_VALUE",
      "TOYLIB_ERR_NOT_INITIALIZED",
      "TOYLIB_ERR_OUT_OF_RANGE",
      "TOYLIB_ERR_CAPACITY_FULL",
      "TOYLIB_ERR_EMPTY",
      "TOYLIB_ERR_NOMEM",
    };

    // 编译时验证names的长度
    _Static_assert(sizeof(names) / sizeof(names[0]) == TOYLIB_ERR_UNKNOWN, "names的长度必须等于TOYLIB_ERR_UNKNOWN");

    // error处于[0, TOYLIB_ERR_UNKNOWN)区间则返回TOYLIB_ERR_UNKNOWN
    if ((int)error < 0 || error >= TOYLIB_ERR_UNKNOWN)
        return "TOYLIB_ERR_UNKNOWN";
    return names[error];
}
