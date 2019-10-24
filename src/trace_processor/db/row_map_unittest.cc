/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "src/trace_processor/db/row_map.h"

#include <memory>

#include "src/base/test/gtest_test_suite.h"
#include "test/gtest_and_gmock.h"

namespace perfetto {
namespace trace_processor {
namespace {

TEST(RowMapUnittest, SmokeRange) {
  RowMap rm(30, 47);

  ASSERT_EQ(rm.size(), 17u);

  ASSERT_EQ(rm.Get(0), 30u);
  ASSERT_EQ(rm.Get(1), 31u);
  ASSERT_EQ(rm.Get(16), 46u);

  ASSERT_EQ(rm.IndexOf(29), base::nullopt);
  ASSERT_EQ(rm.IndexOf(30), 0u);
  ASSERT_EQ(rm.IndexOf(37), 7u);
  ASSERT_EQ(rm.IndexOf(46), 16u);
  ASSERT_EQ(rm.IndexOf(47), base::nullopt);
}

TEST(RowMapUnittest, SmokeBitVector) {
  RowMap rm(BitVector{true, false, false, false, true, true});

  ASSERT_EQ(rm.size(), 3u);

  ASSERT_EQ(rm.Get(0u), 0u);
  ASSERT_EQ(rm.Get(1u), 4u);
  ASSERT_EQ(rm.Get(2u), 5u);

  ASSERT_EQ(rm.IndexOf(0u), 0u);
  ASSERT_EQ(rm.IndexOf(4u), 1u);
  ASSERT_EQ(rm.IndexOf(5u), 2u);

  ASSERT_EQ(rm.IndexOf(1u), base::nullopt);
  ASSERT_EQ(rm.IndexOf(100u), base::nullopt);
}

TEST(RowMapUnittest, SmokeIndexVector) {
  RowMap rm(std::vector<uint32_t>{32u, 56u, 24u, 0u, 100u, 1u});

  ASSERT_EQ(rm.size(), 6u);

  ASSERT_EQ(rm.Get(0u), 32u);
  ASSERT_EQ(rm.Get(1u), 56u);
  ASSERT_EQ(rm.Get(2u), 24u);
  ASSERT_EQ(rm.Get(3u), 0u);
  ASSERT_EQ(rm.Get(4u), 100u);
  ASSERT_EQ(rm.Get(5u), 1u);

  ASSERT_EQ(rm.IndexOf(32u), 0u);
  ASSERT_EQ(rm.IndexOf(56u), 1u);
  ASSERT_EQ(rm.IndexOf(24u), 2u);
  ASSERT_EQ(rm.IndexOf(0u), 3u);
  ASSERT_EQ(rm.IndexOf(100u), 4u);
  ASSERT_EQ(rm.IndexOf(1u), 5u);
}

// TODO(lalitm): add a test here for AddToRangeBefore when we fix the issue
// in RowMap which has incorrect behaviour for this case.

TEST(RowMapUnittest, AddToRangeAfter) {
  RowMap rm(3u, 7u);
  rm.Add(10u);

  ASSERT_EQ(rm.size(), 5u);
  ASSERT_EQ(rm.Get(4u), 10u);
  ASSERT_EQ(rm.IndexOf(10u), 4u);
}

// TODO(lalitm): add a test here for AddToBitVectorBefore when we fix the issue
// in RowMap which has incorrect behaviour for this case.

TEST(RowMapUnittest, AddToBitVectorAfter) {
  RowMap rm(BitVector{true, false, true, true, false, true});
  rm.Add(10u);

  ASSERT_EQ(rm.size(), 5u);
  ASSERT_EQ(rm.Get(4u), 10u);
  ASSERT_EQ(rm.IndexOf(10u), 4u);
}

TEST(RowMapUnittest, AddToIndexVectorAfter) {
  RowMap rm(std::vector<uint32_t>{0u, 2u, 3u, 5u});
  rm.Add(10u);

  ASSERT_EQ(rm.size(), 5u);
  ASSERT_EQ(rm.Get(4u), 10u);
  ASSERT_EQ(rm.IndexOf(10u), 4u);
}

TEST(RowMapUnittest, SelectRangeWithRange) {
  RowMap rm(93, 157);
  RowMap picker(4, 7);
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 3u);
  ASSERT_EQ(res.Get(0u), 97u);
  ASSERT_EQ(res.Get(1u), 98u);
  ASSERT_EQ(res.Get(2u), 99u);
}

TEST(RowMapUnittest, SelectBitVectorWithRange) {
  RowMap rm(BitVector{true, false, false, true, false, true, false});
  RowMap picker(1u, 3u);
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 2u);
  ASSERT_EQ(res.Get(0u), 3u);
  ASSERT_EQ(res.Get(1u), 5u);
}

TEST(RowMapUnittest, SelectIndexVectorWithRange) {
  RowMap rm(std::vector<uint32_t>{33, 2u, 45u, 7u, 8u, 9u});
  RowMap picker(2, 5);
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 3u);
  ASSERT_EQ(res.Get(0u), 45u);
  ASSERT_EQ(res.Get(1u), 7u);
  ASSERT_EQ(res.Get(2u), 8u);
}

TEST(RowMapUnittest, SelectRangeWithBitVector) {
  RowMap rm(27, 31);
  RowMap picker(BitVector{true, false, false, true});
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 2u);
  ASSERT_EQ(res.Get(0u), 27u);
  ASSERT_EQ(res.Get(1u), 30u);
}

TEST(RowMapUnittest, SelectBitVectorWithBitVector) {
  RowMap rm(BitVector{true, false, true, true, false, true});
  RowMap picker(BitVector{true, false, false, true});
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 2u);
  ASSERT_EQ(res.Get(0u), 0u);
  ASSERT_EQ(res.Get(1u), 5u);
}

TEST(RowMapUnittest, SelectIndexVectorWithBitVector) {
  RowMap rm(std::vector<uint32_t>{0u, 2u, 3u, 5u});
  RowMap picker(BitVector{true, false, false, true});
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 2u);
  ASSERT_EQ(res.Get(0u), 0u);
  ASSERT_EQ(res.Get(1u), 5u);
}

TEST(RowMapUnittest, SelectRangeWithIndexVector) {
  RowMap rm(27, 31);
  RowMap picker(std::vector<uint32_t>{3u, 2u, 0u, 1u, 1u, 3u});
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 6u);
  ASSERT_EQ(res.Get(0u), 30u);
  ASSERT_EQ(res.Get(1u), 29u);
  ASSERT_EQ(res.Get(2u), 27u);
  ASSERT_EQ(res.Get(3u), 28u);
  ASSERT_EQ(res.Get(4u), 28u);
  ASSERT_EQ(res.Get(5u), 30u);
}

TEST(RowMapUnittest, SelectBitVectorWithIndexVector) {
  RowMap rm(BitVector{true, false, true, true, false, true});
  RowMap picker(std::vector<uint32_t>{3u, 2u, 0u, 1u, 1u, 3u});
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 6u);
  ASSERT_EQ(res.Get(0u), 5u);
  ASSERT_EQ(res.Get(1u), 3u);
  ASSERT_EQ(res.Get(2u), 0u);
  ASSERT_EQ(res.Get(3u), 2u);
  ASSERT_EQ(res.Get(4u), 2u);
  ASSERT_EQ(res.Get(5u), 5u);
}

TEST(RowMapUnittest, SelectIndexVectorWithIndexVector) {
  RowMap rm(std::vector<uint32_t>{33u, 2u, 45u, 7u, 8u, 9u});
  RowMap picker(std::vector<uint32_t>{3u, 2u, 0u, 1u, 1u, 3u});
  auto res = rm.SelectRows(picker);

  ASSERT_EQ(res.size(), 6u);
  ASSERT_EQ(res.Get(0u), 7u);
  ASSERT_EQ(res.Get(1u), 45u);
  ASSERT_EQ(res.Get(2u), 33u);
  ASSERT_EQ(res.Get(3u), 2u);
  ASSERT_EQ(res.Get(4u), 2u);
  ASSERT_EQ(res.Get(5u), 7u);
}

TEST(RowMapUnittest, RemoveIfRange) {
  RowMap rm(27u, 31u);
  rm.RemoveIf([](uint32_t row) { return row == 27u || row == 29u; });

  ASSERT_EQ(rm.size(), 2u);
  ASSERT_EQ(rm.Get(0u), 28u);
  ASSERT_EQ(rm.Get(1u), 30u);
}

TEST(RowMapUnittest, RemoveIfBitVector) {
  RowMap rm(BitVector{true, false, true, true, false, true});
  rm.RemoveIf([](uint32_t row) { return row == 2u || row == 5u; });

  ASSERT_EQ(rm.size(), 2u);
  ASSERT_EQ(rm.Get(0u), 0u);
  ASSERT_EQ(rm.Get(1u), 3u);
}

TEST(RowMapUnittest, RemoveIfIndexVector) {
  RowMap rm(std::vector<uint32_t>{3u, 2u, 0u, 1u, 1u, 3u});
  rm.RemoveIf([](uint32_t row) { return row == 3u; });

  ASSERT_EQ(rm.size(), 4u);
  ASSERT_EQ(rm.Get(0u), 2u);
  ASSERT_EQ(rm.Get(1u), 0u);
  ASSERT_EQ(rm.Get(2u), 1u);
  ASSERT_EQ(rm.Get(3u), 1u);
}

}  // namespace
}  // namespace trace_processor
}  // namespace perfetto
