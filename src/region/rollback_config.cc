// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/rollback_config.h"

#include <format>
#include <string>

namespace region {

namespace {

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
    return "\"undefined\"";
  }
}

}  // namespace

std::string format_rollback_config(const RollbackConfig& config) {
  return std::format(
      R"(
[rollback_config]

src_world = "{}"
dest_world = "{}"
dim = "{}"
type = "{}"
color = "{}"
src_world_structure = "{}"
dest_world_structure = "{}"
chunks_list = {}
min_x = {}
max_x = {}
min_z = {}
max_z = {}
num_threads = {}
verbose = {}

)",
      config.src_world, config.dest_world, config.dim_str, config.type_str,
      config.color_str, config.src_world_structure_str,
      config.dest_world_structure_str, format_chunks(config.chunks),
      format_opt_int(config.min_x), format_opt_int(config.max_x),
      format_opt_int(config.min_z), format_opt_int(config.max_z),
      config.num_threads, config.verbose);
}

}  // namespace region
