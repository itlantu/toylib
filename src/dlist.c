#include "toylib/dlist.h"

#include <stdlib.h>

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