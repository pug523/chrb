// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "core/core.h"
#include "region/chunk_range.h"
#include "region/dimension.h"
#include "region/region_position.h"
#include "region/rollback_type.h"

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
