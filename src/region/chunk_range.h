// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "core/core.h"

namespace region {

struct ChunkRange {
  i32 min_x;
  i32 min_z;
  i32 max_x;
  i32 max_z;
};

}  // namespace region
