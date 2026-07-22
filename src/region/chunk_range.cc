// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/chunk_range.h"

#include <format>
#include <string>

namespace region {

std::string dump_chunk_range(const ChunkRange& chunk_range) {
  return std::format("{}.{} ~ {}.{}", chunk_range.min_x, chunk_range.min_z,
                     chunk_range.max_x, chunk_range.max_z);
}

}  // namespace region

