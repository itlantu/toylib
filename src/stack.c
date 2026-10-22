#include "toylib/stack.h"

#include <stdlib.h>

ToylibError stack_init(Stack* stack, const size_t capacity) {
    if (stack == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (capacity > TOYLIB_STACK_MAX_CAPACITY)
        return TOYLIB_ERR_BAD_ARG_2;

    TOYLIB_STACK_VALUE_TYPE* data = malloc(sizeof(TOYLIB_STACK_VALUE_TYPE) * capacity);
    if (data == NULL)
        return TOYLIB_ERR_NOMEM;

    stack->capacity_ = capacity;
    stack->length_ = 0;
    stack->data_ = data;
    return TOYLIB_OK;
}

ToylibError stack_push(Stack* stack, const TOYLIB_STACK_VALUE_TYPE value) {
    if (stack == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (stack->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    const size_t length = stack->length_;
    if (length == stack->capacity_)
        return TOYLIB_ERR_CAPACITY_FULL;

    stack->data_[length] = value;
    stack->length_ += 1;
    return TOYLIB_OK;
}

ToylibError stack_pop(Stack* stack, TOYLIB_STACK_VALUE_TYPE* value) {
    const ToylibError err = stack_top(stack, value);
    if (err != TOYLIB_OK)
        return err;
    stack->length_ -= 1;

    return TOYLIB_OK;
}

ToylibError stack_top(Stack* stack, TOYLIB_STACK_VALUE_TYPE* value) {
    if (stack == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (value == NULL)
        return TOYLIB_ERR_BAD_ARG_2;
    if (stack->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    if (!stack->length_)
        return TOYLIB_ERR_EMPTY;

    *value = stack->data_[stack->length_ - 1];
    return TOYLIB_OK;
}

ToylibError stack_free(Stack* stack) {
    if (stack == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (stack->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    free(stack->data_);
    stack->length_ = 0;
    stack->capacity_ = 0;
    stack->data_ = NULL;
    return TOYLIB_OK;
}

ToylibError stack_length(Stack* stack, size_t* length) {
    if (stack == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (length == NULL)
        return TOYLIB_ERR_BAD_ARG_2;

    *length = stack->length_;
    return TOYLIB_OK;
}
