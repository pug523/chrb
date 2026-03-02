// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <fstream>

#include "core/core.h"

namespace core {

struct LocationEntry {
  u32 offset = 0;
  u8 sectors = 0;
};

LocationEntry read_location(std::fstream& file, i32 index);

}  // namespace core
