#include "toylib/queue.h"

#include <stdlib.h>

ToylibError queue_init(Queue* queue, const size_t capacity) {
    if (queue == NULL)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (capacity == 0 || capacity > TOYLIB_QUEUE_MAX_SIZE)
        return TOYLIB_ERR_BAD_ARG_VALUE;

    queue->data_ = malloc(sizeof(TOYLIB_QUEUE_VALUE_TYPE) * capacity);
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOMEM;

    queue->capacity_ = capacity;
    queue->front_ = 0;
    queue->rear_ = 0;
    queue->length_ = 0;
    return TOYLIB_OK;
}

ToylibError queue_push(Queue* queue, const TOYLIB_QUEUE_VALUE_TYPE value) {
    if (queue == NULL)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    if (queue->length_ == queue->capacity_)
        return TOYLIB_ERR_CAPACITY_FULL;

    const size_t rear = queue->rear_;
    queue->data_[rear] = value;
    queue->rear_ = (rear + 1) % queue->capacity_;
    queue->length_ += 1;

    return TOYLIB_OK;
}

ToylibError queue_pop(Queue* queue, TOYLIB_QUEUE_VALUE_TYPE* value) {
    if (queue == NULL)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    if (queue->length_ == 0)
        return TOYLIB_ERR_EMPTY;

    const size_t front = queue->front_;
    if (value != NULL)
        *value = queue->data_[front];
    queue->front_ = (front + 1) % queue->capacity_;
    queue->length_ -= 1;

    return TOYLIB_OK;
}

ToylibError queue_front(const Queue* queue, TOYLIB_QUEUE_VALUE_TYPE* value) {
    if (queue == NULL || value)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    *value = queue->data_[queue->front_];

    return TOYLIB_OK;
}

ToylibError queue_rear(const Queue* queue, TOYLIB_QUEUE_VALUE_TYPE* value) {
    if (queue == NULL || value == NULL)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    *value = queue->data_[queue->rear_];

    return TOYLIB_OK;
}

ToylibError queue_free(Queue* queue) {
    if (queue == NULL)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    free(queue->data_);
    queue->data_ = NULL;
    queue->capacity_ = 0;
    queue->front_ = 0;
    queue->rear_ = 0;
    queue->length_ = 0;

    return TOYLIB_OK;
}

ToylibError queue_length(const Queue* queue, size_t* length) {
    if (queue == NULL || length == NULL)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    *length = queue->length_;

    return TOYLIB_OK;
}

ToylibError queue_capacity(const Queue* queue, size_t* capacity) {
    if (queue == NULL || capacity == NULL)
        return TOYLIB_ERR_BAD_ARG_NULL;
    if (queue->data_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    *capacity = queue->capacity_;

    return TOYLIB_OK;
}
