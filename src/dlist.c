#include "toylib/dlist.h"

#include <stdlib.h>

static DListNode* dlist_index_node_(const DList* dlist, const size_t index) {
    const DListNode* head = dlist->head_;
    const DListNode* tail = dlist->tail_;
    const size_t length = dlist->length_;

    DListNode* p = head->next_;

    if (index <= length / 2) {
        // 下标离头指针近, 从头指针开始找
        for (size_t i = 0; i < index; ++i) {
            p = p->next_;
        }
        return p;
    }
    // 下标离尾指针近, 从尾指针开始往前找
    p = tail->prev_;
    for (size_t i = index + 1; i < length; ++i) {
        p = p->prev_;
    }

    return p;
}

static DListNode* dlist_node_create(const TOYLIB_DLIST_VALUE_TYPE value, DListNode* prev, DListNode* next) {
    DListNode* result = (DListNode*)malloc(sizeof(DListNode));
    result->value_ = value;
    result->prev_ = prev;
    result->next_ = next;
    return result;
}

ToylibError dlist_init(DList* dlist) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;

    dlist->head_ = dlist_node_create(0, NULL, NULL);
    dlist->tail_ = dlist_node_create(0, NULL, NULL);
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOMEM;

    dlist->head_->next_ = dlist->tail_;
    dlist->tail_->prev_ = dlist->head_;

    dlist->length_ = 0;
    return TOYLIB_OK;
}

ToylibError dlist_insert(DList* dlist, const size_t index, TOYLIB_DLIST_VALUE_TYPE value) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    if (index > dlist->length_)
        return TOYLIB_ERR_OUT_OF_RANGE;

    DListNode* p = dlist->head_;
    for (size_t i = 0; i < index; ++i) {
        p = p->next_;
    }

    // 插入
    DListNode* prev_node = p;
    DListNode* next_node = p->next_;
    DListNode* insert_node = dlist_node_create(value, prev_node, next_node);
    if (insert_node == NULL)
        return TOYLIB_ERR_NOMEM;

    prev_node->next_ = insert_node;
    next_node->prev_ = insert_node;
    dlist->length_ += 1;

    return TOYLIB_OK;
}

ToylibError dlist_delete(DList* dlist, const size_t index, TOYLIB_DLIST_VALUE_TYPE* deleted_value) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    if (dlist->head_->next_ == dlist->tail_)
        return TOYLIB_ERR_EMPTY;
    if (index >= dlist->length_)
        return TOYLIB_ERR_OUT_OF_RANGE;

    DListNode* p = dlist->head_;
    for (size_t i = 0; i <= index; ++i) {
        p = p->next_;
    }

    if (deleted_value != NULL)
        *deleted_value = p->value_;
    // 移除
    DListNode* prev_node = p->prev_;
    DListNode* next_node = p->next_;
    free(p);

    prev_node->next_ = next_node;
    next_node->prev_ = prev_node;
    dlist->length_ -= 1;

    return TOYLIB_OK;
}

ToylibError dlist_find(const DList* dlist, size_t* index, const TOYLIB_DLIST_VALUE_TYPE value) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (index == NULL)
        return TOYLIB_ERR_BAD_ARG_2;
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    DListNode* p = dlist->head_->next_;
    const DListNode* tail = dlist->tail_;
    *index = TOYLIB_DLIST_VALUE_NOT_FOUND;

    for (size_t i = 0; p != tail; ++i) {
        if (TOYLIB_DLIST_VALUE_IS_EQUAL(p->value_, value)) {
            *index = i;
            break;
        }
        p = p->next_;
    }
    return TOYLIB_OK;
}

ToylibError dlist_free(DList* dlist) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;

    DListNode* p = dlist->head_;

    while (p != NULL) {
        DListNode* next = p->next_;
        free(p);
        p = next;
    }
    dlist->head_ = NULL;
    dlist->tail_ = NULL;
    dlist->length_ = 0;

    return TOYLIB_OK;
}

ToylibError dlist_get(const DList* dlist, const size_t index, TOYLIB_DLIST_VALUE_TYPE* value) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (value == NULL)
        return TOYLIB_ERR_BAD_ARG_3;
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    if (index >= dlist->length_)
        return TOYLIB_ERR_OUT_OF_RANGE;

    // DListNode* p = dlist->head_->next_;
    // for (size_t i = 0; i < index; ++i) {
    //     p = p->next_;
    // }
    const DListNode* p = dlist_index_node_(dlist, index);
    *value = p->value_;

    return TOYLIB_OK;
}

ToylibError dlist_set(DList* dlist, const size_t index, TOYLIB_DLIST_VALUE_TYPE value) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    if (index >= dlist->length_)
        return TOYLIB_ERR_OUT_OF_RANGE;

    // DListNode* p = dlist->head_->next_;
    // for (size_t i = 0; i < index; ++i) {
    //     p = p->next_;
    // }
    DListNode* p = dlist_index_node_(dlist, index);
    p->value_ = value;
    return TOYLIB_OK;
}

ToylibError dlist_length(const DList* dlist, size_t* length) {
    if (dlist == NULL)
        return TOYLIB_ERR_BAD_ARG_1;
    if (length == NULL)
        return TOYLIB_ERR_BAD_ARG_2;
    if (dlist->head_ == NULL || dlist->tail_ == NULL)
        return TOYLIB_ERR_NOT_INITIALIZED;
    *length = dlist->length_;
    return TOYLIB_OK;
}
