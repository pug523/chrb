// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/rollback_config.h"

#include <charconv>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "core/format_util.h"
#include "region/chunk_position.h"

namespace region {

std::string dump_rollback_config(const RollbackConfig& config) {
  return std::format(
      R"(
[rollback_config]

src_world = "{}"
dest_world = "{}"
dimension = "{}"
rollback_type = "{}"
color = "{}"
src_world_structure = "{}"
dest_world_structure = "{}"
chunks = {}
min_x = {}
max_x = {}
min_z = {}
max_z = {}
num_threads = {}
allow_whole_rollback = {}
bulk_copy = {}
dry_run = {}
silent = {}
verbose = {}
)",
      config.src_world, config.dest_world, config.dim_str, config.type_str,
      config.color_str, config.src_world_structure_str,
      config.dest_world_structure_str, core::format_chunks(config.chunks),
      core::format_opt_int(config.min_x), core::format_opt_int(config.max_x),
      core::format_opt_int(config.min_z), core::format_opt_int(config.max_z),
      config.num_threads, config.allow_whole_rollback, config.bulk_copy,
      config.dry_run, config.silent, config.verbose);
}

std::vector<ChunkPosition> parse_chunks(
    const std::vector<std::vector<i32>>& chunks) {
  std::vector<ChunkPosition> result;
  result.resize(chunks.size());
  for (usize i = 0; i < chunks.size(); ++i) {
    const std::vector<i32>& chunk = chunks[i];
    if (chunk.size() != 2) {
      continue;
    }
    result[i].x = chunk[0];
    result[i].z = chunk[1];
  }
  return result;
}

std::vector<ChunkPosition> parse_chunks(
    const std::vector<std::string_view>& raw_chunks) {
  std::vector<std::vector<i32>> chunks;
  chunks.resize(raw_chunks.size());
  for (usize i = 0; i < chunks.size(); ++i) {
    const std::string_view chunk = raw_chunks[i];
    const usize dot = chunk.find_first_of('.');
    if (dot == std::string_view::npos) {
      continue;
    }
    const std::string_view x = chunk.substr(0, dot);
    const std::string_view z = chunk.substr(dot + 1);
    i32 ix = 0;
    i32 iz = 0;
    auto [xptr, xec] = std::from_chars(x.data(), x.data() + x.size(), ix);
    auto [zptr, zec] = std::from_chars(z.data(), z.data() + z.size(), iz);
    if (xec != std::errc() || zec != std::errc()) {
      continue;
    }
    chunks[i] = std::vector<i32>{ix, iz};
  }
  return parse_chunks(chunks);
}

}  // namespace region
