// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "core/core.h"
#include "core/region/chunk_range.h"
#include "core/region/dimension.h"
#include "core/region/region_position.h"
#include "core/region/rollback_type.h"

namespace core {

enum class RollbackMode : u8 {
  FullCopy,
  Partial,
};

struct RollbackTask {
  RegionPosition region;
  ChunkRange chunk_range;
  Dimension dimension;
  RollbackType type;
  RollbackMode mode;
};

}  // namespace core
