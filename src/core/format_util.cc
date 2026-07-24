// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/format_util.h"

#include <cstddef>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "region/chunk_position.h"

namespace core {

std::string format_chunks(const std::vector<region::ChunkPosition>& chunks) {
  std::string result;
  constexpr size_t kMargin = 8;
  constexpr size_t kStrLengthPerChunk = 12;
  result.resize(chunks.size() * kStrLengthPerChunk + kMargin);
  result.append("[\n");
  for (region::ChunkPosition chunk : chunks) {
    result.append(std::format("  [{}, {}],\n", chunk.x, chunk.z));
  }
  result.append("]");
  return result;
}

std::string format_opt_int(std::optional<i32> i) {
  if (i.has_value()) {
    return std::format("{}", i.value());
  } else {
    return "\"none\"";
  }
}

}  // namespace core

