// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/chunk_range.h"

#include "catch2/catch_test_macros.hpp"

TEST_CASE("ChunkRange formatting", "[region][chunk_range]") {
  SECTION("dump_chunk_range formats range correctly") {
    const region::ChunkRange range{
        .min_x = -10, .min_z = 5, .max_x = 20, .max_z = 30};
    REQUIRE(region::dump_chunk_range(range) == "-10.5 ~ 20.30");
  }

  SECTION("dump_chunk_range handles zero and negative boundaries") {
    const region::ChunkRange range{
        .min_x = 0, .min_z = 0, .max_x = 0, .max_z = 0};
    REQUIRE(region::dump_chunk_range(range) == "0.0 ~ 0.0");
  }
}
