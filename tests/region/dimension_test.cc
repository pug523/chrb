// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/dimension.h"

#include <string_view>

#include "catch2/catch_test_macros.hpp"

TEST_CASE("Dimension string conversion", "[region][dimension]") {
  SECTION("str_to_dimension converts valid string to Dimension enum") {
    REQUIRE(region::str_to_dimension("overworld") ==
            region::Dimension::OverWorld);
    REQUIRE(region::str_to_dimension("nether") == region::Dimension::Nether);
    REQUIRE(region::str_to_dimension("end") == region::Dimension::End);
  }

  SECTION("str_to_dimension converts invalid or uppercase string to Unknown") {
    REQUIRE(region::str_to_dimension("OVERWORLD") ==
            region::Dimension::Unknown);
    REQUIRE(region::str_to_dimension("invalid") == region::Dimension::Unknown);
    REQUIRE(region::str_to_dimension("") == region::Dimension::Unknown);
  }

  SECTION("dimension_to_str converts Dimension enum to correct string") {
    REQUIRE(std::string_view(region::dimension_to_str(
                region::Dimension::OverWorld)) == "overworld");
    REQUIRE(std::string_view(region::dimension_to_str(
                region::Dimension::Nether)) == "nether");
    REQUIRE(std::string_view(
                region::dimension_to_str(region::Dimension::End)) == "end");
  }

  SECTION("dimension_path_with_slash functions return valid path prefixes") {
    REQUIRE(std::string_view(region::dimension_path_with_slash_old(
                region::Dimension::Nether)) == "DIM-1/");
    REQUIRE(std::string_view(region::dimension_path_with_slash_new(
                region::Dimension::Nether)) ==
            "dimensions/minecraft/the_nether/");
    REQUIRE(std::string_view(region::dimension_path_with_slash_paper(
                region::Dimension::Nether)) == "world_nether/DIM-1/");
  }
}
