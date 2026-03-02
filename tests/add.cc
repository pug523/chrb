// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>

TEST_CASE("simple addition") {
  REQUIRE((1 + 1) == 2);
}
