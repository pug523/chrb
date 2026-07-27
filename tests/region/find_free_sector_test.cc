// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/find_free_sector.h"

#include <vector>

#include "catch2/catch_test_macros.hpp"

TEST_CASE("find_free_sector search algorithm", "[region][find_free_sector]") {
  SECTION("returns -1 when needed count is zero or vector is too small") {
    const std::vector<bool> used(2, true);
    REQUIRE(region::find_free_sector(used, 1) == -1);
  }

  SECTION("finds first available block of specified size after sector 2") {
    // Indices: 0: used, 1: used, 2: used, 3: free, 4: free, 5: free, 6: used
    const std::vector<bool> used = {true,  true,  true, false,
                                    false, false, true};

    REQUIRE(region::find_free_sector(used, 1) == 3);
    REQUIRE(region::find_free_sector(used, 2) == 3);
    REQUIRE(region::find_free_sector(used, 3) == 3);
    REQUIRE(region::find_free_sector(used, 4) == -1);
  }

  SECTION("skips header sectors 0 and 1 even if they are marked free") {
    // Indices 0 and 1 are free in vector, but algorithm must start searching
    // from index 2
    const std::vector<bool> used = {false, false, true, true, false, false};

    REQUIRE(region::find_free_sector(used, 2) == 4);
  }

  SECTION("returns -1 when no contiguous free space exists") {
    const std::vector<bool> used = {true, true, false, true, false, true};
    REQUIRE(region::find_free_sector(used, 2) == -1);
  }
}
