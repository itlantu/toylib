/*
 * L3场景级, 用std::vector当参考模型, 跑随机操作序列逐步比对
 *
 * 单步都对的函数组合起来仍可能出错(比如删除后插入、连续删除到空),
 * 这类问题只有多步序列才暴露。每一步都和模型全量比对, 保证跑偏时立刻停下。
 */

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "seqlist_support.h"

using toylib_test::Element;
using toylib_test::to_vector;

namespace {

/**
 * 生成一个偏向边界的下标
 * @param rng 随机数引擎
 * @param length 当前长度
 * @return 下标, 大部分落在合法区间, 少数越界
 * @note 均匀随机会很少碰到0/length/SIZE_MAX这些关键点, 这里刻意加权
 */
size_t pick_index(std::mt19937& rng, const size_t length) {
    switch (rng() % 6) {
        case 0:
            return 0;
        case 1:
            return length == 0 ? 0 : length - 1;
        case 2:
            return length;
        case 3:
            return length + 1;
        case 4:
            return SIZE_MAX;
        default:
            return length == 0 ? 0 : rng() % (length + 1);
    }
}

}  // namespace

TEST_SUITE_BEGIN("[SeqList][L3]");

/**
 * 校验随机操作序列下, 顺序表的行为和参考模型逐步一致
 * @note 种子固定, 失败时按step就能复现; 改种子可以探索另一条路径
 */
TEST_CASE("random operations match the model") {
    constexpr size_t kCapacity = 16;
    constexpr int kSteps = 500;
    constexpr unsigned kSeed = 20261021u;

    std::mt19937 rng(kSeed);
    std::vector<Element> model;

    SeqList list{};
    REQUIRE_EQ(TOYLIB_OK, seq_list_create(&list, kCapacity));

    for (int step = 0; step < kSteps; ++step) {
        const int op = static_cast<int>(rng() % 5);
        const size_t index = pick_index(rng, model.size());
        const Element value = static_cast<Element>(rng() % 10);

        std::string op_name = "insert";
        if (op == 1)
            op_name = "delete";
        else if (op == 2)
            op_name = "find";
        else if (op == 3)
            op_name = "get";
        else if (op == 4)
            op_name = "set";
        CAPTURE(step);
        CAPTURE(op_name);
        CAPTURE(index);
        CAPTURE(value);

        switch (op) {
            case 0: {
                // insert: 先判下标再判容量
                ToylibError expected = TOYLIB_OK;
                if (index > model.size())
                    expected = TOYLIB_ERR_OUT_OF_RANGE;
                else if (model.size() == kCapacity)
                    expected = TOYLIB_ERR_CAPACITY_FULL;

                CHECK_EQ(expected, seq_list_insert(&list, index, value));
                if (expected == TOYLIB_OK)
                    model.insert(model.begin() + static_cast<std::ptrdiff_t>(index), value);
                break;
            }
            case 1: {
                // delete: 先判空表再判下标
                ToylibError expected = TOYLIB_OK;
                if (model.empty())
                    expected = TOYLIB_ERR_EMPTY;
                else if (index >= model.size())
                    expected = TOYLIB_ERR_OUT_OF_RANGE;

                Element removed = 0;
                CHECK_EQ(expected, seq_list_delete(&list, index, &removed));
                if (expected == TOYLIB_OK) {
                    CHECK_EQ(removed, model[index]);
                    model.erase(model.begin() + static_cast<std::ptrdiff_t>(index));
                }
                break;
            }
            case 2: {
                // find: 查不到时出参是哨兵
                size_t found = 0;
                CHECK_EQ(TOYLIB_OK, seq_list_find(&list, &found, value));

                size_t expected = TOYLIB_SEQLIST_ELEMENT_NOT_FOUND;
                for (size_t i = 0; i < model.size(); ++i) {
                    if (model[i] == value) {
                        expected = i;
                        break;
                    }
                }
                CHECK_EQ(found, expected);
                break;
            }
            case 3: {
                // get: 下标必须落在[0, length)
                ToylibError expected = index >= model.size() ? TOYLIB_ERR_OUT_OF_RANGE : TOYLIB_OK;

                Element got = 0;
                CHECK_EQ(expected, seq_list_get(&list, index, &got));
                if (expected == TOYLIB_OK)
                    CHECK_EQ(got, model[index]);
                break;
            }
            default: {
                // set: 边界和get完全一样, 只是写而不是读
                ToylibError expected = index >= model.size() ? TOYLIB_ERR_OUT_OF_RANGE : TOYLIB_OK;

                CHECK_EQ(expected, seq_list_set(&list, index, value));
                if (expected == TOYLIB_OK)
                    model[index] = value;
                break;
            }
        }

        // 每一步之后全量比对, 跑偏了立刻停下来
        CHECK_EQ(to_vector(list), model);

        size_t length = 0;
        CHECK_EQ(TOYLIB_OK, seq_list_length(&list, &length));
        CHECK_EQ(length, model.size());
    }

    seq_list_free(&list);
}

TEST_SUITE_END();
