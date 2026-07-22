// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/rollback_task.h"

#include <format>
#include <string>

#include "core/format_util.h"
#include "region/chunk_range.h"
#include "region/dimension.h"
#include "region/rollback_type.h"

namespace region {

std::string dump_task(const RollbackTask& task) {
  return std::format(
      R"([task]
region = "{}.{}"
target_chunks = {}
chunk_range = "{}"
dimension = "{}"
rollback_type = "{}"
rollback_mode = "{}"
)",
      task.region.x, task.region.z, core::format_chunks(task.target_chunks),
      dump_chunk_range(task.chunk_range), dimension_to_str(task.dimension),
      type_to_str(task.type), mode_to_str(task.mode));
}

}  // namespace region
