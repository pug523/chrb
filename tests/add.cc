// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "catch2/catch_test_macros.hpp"
#define CATCH_CONFIG_MAIN

TEST_CASE("simple addition") {
  REQUIRE((1 + 1) == 2);
}
