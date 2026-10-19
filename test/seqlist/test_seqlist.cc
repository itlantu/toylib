#include "doctest/doctest.h"
#include "toylib/seqlist.h"

#include <vector>

using namespace std;

namespace {

TEST_SUITE("SeqList") {
    TEST_CASE("Basic: seq_list_init") {
        SeqList seq_list;
        constexpr size_t capacity = 1024;
        auto* data = new TOYLIB_SEQLIST_ELEMENT_TYPE[capacity];

        seq_list_init(&seq_list, data, capacity);

        CHECK_EQ(capacity, seq_list.capacity_);
        CHECK_EQ(data, seq_list.data_);
        CHECK_EQ(0, seq_list.length_);
    }

    TEST_CASE("Basic: seq_list_create") {
        SeqList seq_list;
        constexpr size_t capacity = 16;

        seq_list_create(&seq_list, capacity);

        CHECK_NE(seq_list.data_, nullptr);
        CHECK_EQ(seq_list.capacity_, capacity);
    }

    TEST_CASE("Easy: seq_list_insert") {
        SeqList seq_list;
        constexpr size_t capacity = 16;
        seq_list_create(&seq_list, capacity);

        auto* data = seq_list.data_;

        seq_list_insert(&seq_list, 0, 0);
        CHECK_EQ(data[0], 0);

        seq_list_insert(&seq_list, 1, 2);
        CHECK_EQ(data[0], 0);
        CHECK_EQ(data[1], 2);

        seq_list_insert(&seq_list, 1, 1);
        CHECK_EQ(data[0], 0);
        CHECK_EQ(data[1], 1);
        CHECK_EQ(data[2], 2);

        CHECK_EQ(TOYLIB_ERR_OUT_OF_RANGE, seq_list_insert(&seq_list, 4, 1024));
    }

    TEST_CASE("Easy: seq_list_delete") {
        SeqList seq_list;

        vector<TOYLIB_SEQLIST_ELEMENT_TYPE> arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        TOYLIB_SEQLIST_ELEMENT_TYPE element = 0;
        seq_list_create(&seq_list, arr.size());

        for (size_t i = 0; i < arr.size(); ++i) {
            seq_list_insert(&seq_list, i, arr[i]);
        }

        arr.erase(arr.begin() + 5);
        seq_list_delete(&seq_list, 5, &element);

        CHECK_EQ(element, 6);
        for (size_t i = 0; i < arr.size(); ++i)
            CHECK_EQ(arr[i], seq_list.data_[i]);
    }
}

};