#include "toylib/seqlist.h"
#include <stdlib.h>

ToylibError seq_list_create(SeqList* seq_list, const size_t capacity) {
    if (seq_list == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (capacity == 0 || capacity > TOYLIB_SEQLIST_MAX_CAPACITY)
        return TOYLIB_ERR_BAD_ARG_2;

    void* data = malloc(sizeof(TOYLIB_SEQLIST_ELEMENT_TYPE) * capacity);
    if (data == NULL)
        return TOYLIB_ERR_NOMEM;

    seq_list->capacity_ = capacity;
    seq_list->length_ = 0;
    seq_list->data_ = data;
    return TOYLIB_OK;
}

ToylibError seq_list_init(SeqList* seq_list, TOYLIB_SEQLIST_ELEMENT_TYPE* data, const size_t capacity) {
    if (seq_list == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (data == NULL)
        return TOYLIB_ERR_BAD_ARG_2;
    if (capacity == 0 || capacity > TOYLIB_SEQLIST_MAX_CAPACITY)
        return TOYLIB_ERR_BAD_ARG_3;

    seq_list->capacity_ = capacity;
    seq_list->length_ = 0;
    seq_list->data_ = data;
    return TOYLIB_OK;
}

ToylibError seq_list_insert(SeqList* seq_list, const size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE element) {
    const size_t length = seq_list->length_;

    if (seq_list == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (index > length)
        return TOYLIB_ERR_OUT_OF_RANGE;
    if (length == seq_list->capacity_)
        return TOYLIB_ERR_CAPACITY_FULL;

    TOYLIB_SEQLIST_ELEMENT_TYPE* data = seq_list->data_;

    for (size_t i = length; i > index; --i)
        data[i] = data[i - 1];
    data[index] = element;
    seq_list->length_ += 1;

    return TOYLIB_OK;
}

ToylibError seq_list_delete(SeqList* seq_list, const size_t index, TOYLIB_SEQLIST_ELEMENT_TYPE* deleted_element) {
    const size_t length = seq_list->length_;

    if (seq_list == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (index >= length)
        return TOYLIB_ERR_OUT_OF_RANGE;

    TOYLIB_SEQLIST_ELEMENT_TYPE* data = seq_list->data_;
    const TOYLIB_SEQLIST_ELEMENT_TYPE element = data[index];

    for (size_t i = index; i <= length; ++i) {
        data[i] = data[i + 1];
    }
    seq_list->length_ -= 1;

    if (deleted_element != NULL)
        *deleted_element = element;
    return TOYLIB_OK;
}

ToylibError seq_list_free(SeqList* seq_list) {
    if (seq_list == NULL)
        return TOYLIB_ERR_BAD_ARG_1;

    free(seq_list->data_);
    seq_list->capacity_ = 0;
    seq_list->length_ = 0;
    
    return TOYLIB_OK;
}