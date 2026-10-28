/*
 * L3场景级, 用std::vector当参考模型, 跑随机操作序列逐步比对
 *
 * 双链表的状态更多(head_/tail_/length_三条线索), 单步测试只能覆盖到某一条,
 * 随机序列能把"插入后删除""连续删除到空""删空再插入"这些组合都走一遍。
 */

#include <iterator>
#include <random>
#include <vector>

#include "dlist_support.h"
#include "doctest/doctest.h"

using toylib_test::check_sentinels_outward;
using toylib_test::kOpNames;
using toylib_test::make_dlist;
using toylib_test::pick_boundary_index;
using toylib_test::to_model;
using toylib_test::to_model_reversed;
using toylib_test::to_values;
using toylib_test::to_values_reversed;
using toylib_test::Value;

TEST_SUITE_BEGIN("[DList][L3]");

/**
 * 校验随机操作序列下, 双链表的行为和参考模型逐步一致
 * @note 种子固定, 失败时按step就能复现; 改种子可以探索另一条路径
 */
TEST_CASE("random operations match the model") {
    constexpr int kSteps = 500;
    constexpr unsigned kSeed = 20261021u;

    std::mt19937 rng(kSeed);
    std::vector<Value> model;

    DList dlist = make_dlist();

    for (int step = 0; step < kSteps; ++step) {
        const int op = static_cast<int>(rng() % std::size(kOpNames));
        const size_t index = pick_boundary_index(rng, model.size());
        const Value value = static_cast<Value>(rng() % 10);

        CAPTURE(step);
        CAPTURE(kOpNames[op]);
        CAPTURE(index);
        CAPTURE(value);

        switch (op) {
            case 0: {
                // insert: 插入位置的上界是length
                ToylibError expected = index > model.size() ? TOYLIB_ERR_OUT_OF_RANGE : TOYLIB_OK;

                CHECK_EQ(expected, dlist_insert(&dlist, index, value));
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

                Value removed = 0;
                CHECK_EQ(expected, dlist_delete(&dlist, index, &removed));
                if (expected == TOYLIB_OK) {
                    CHECK_EQ(removed, model[index]);
                    model.erase(model.begin() + static_cast<std::ptrdiff_t>(index));
                }
                break;
            }
            case 2: {
                // find: 查不到时出参是哨兵
                size_t found = 0;
                CHECK_EQ(TOYLIB_OK, dlist_find(&dlist, &found, value));

                size_t expected = TOYLIB_DLIST_VALUE_NOT_FOUND;
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

                Value got = 0;
                CHECK_EQ(expected, dlist_get(&dlist, index, &got));
                if (expected == TOYLIB_OK)
                    CHECK_EQ(got, model[index]);
                break;
            }
            default: {
                // set: 边界和get完全一样, 只是写而不是读
                ToylibError expected = index >= model.size() ? TOYLIB_ERR_OUT_OF_RANGE : TOYLIB_OK;

                CHECK_EQ(expected, dlist_set(&dlist, index, value));
                if (expected == TOYLIB_OK)
                    model[index] = value;
                break;
            }
        }

        // 每一步之后全量比对内容和长度
        CHECK_EQ(to_values(dlist), to_model(model));

        size_t length = 0;
        CHECK_EQ(TOYLIB_OK, dlist_length(&dlist, &length));
        CHECK_EQ(length, model.size());

        // 反向链和哨兵的不变量: 只核对正向顺序发现不了prev_接错
        CHECK_EQ(to_values_reversed(dlist), to_model_reversed(model));
        check_sentinels_outward(dlist);
    }

    dlist_free(&dlist);
}

TEST_SUITE_END();
