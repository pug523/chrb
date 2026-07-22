// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <vector>

#include "core/core.h"
#include "region/chunk_position.h"
#include "region/chunk_range.h"
#include "region/dimension.h"
#include "region/region_position.h"
#include "region/rollback_type.h"

namespace region {

enum class RollbackMode : u8 {
  FullCopy,
  Partial,
};

inline const char* mode_to_str(RollbackMode m) {
  switch (m) {
    case RollbackMode::FullCopy: return "full_copy";
    case RollbackMode::Partial: return "partial";
    default: DCHECK(false); return "unknown";
  }
}

struct RollbackTask {
  RegionPosition region;
  std::vector<ChunkPosition> target_chunks;
  ChunkRange chunk_range;
  Dimension dimension;
  RollbackType type;
  RollbackMode mode;
};

std::string dump_task(const RollbackTask& task);

}  // namespace region
