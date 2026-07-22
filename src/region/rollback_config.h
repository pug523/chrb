// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "region/chunk_position.h"
#include "region/dimension.h"
#include "region/rollback_type.h"
#include "region/world_directory_structure.h"

namespace region {

// Note: when update this struct update also `src/app/args/build_parser.cc`,
// `src/app/toml/toml_parser.cc` and `region/rollback_config.cc`
struct RollbackConfig {
  std::string src_world = "undefined";
  std::string dest_world = "undefined";
  std::string dim_str = "overworld";
  Dimension dimension = Dimension::OverWorld;
  std::string type_str = "all";
  RollbackType type = RollbackType::All;
  std::string color_str = "auto";
  core::ColorMode color_mode = core::ColorMode::Auto;
  std::string src_world_structure_str = "auto";
  WorldDirectoryStructureConfig src_world_structure =
      WorldDirectoryStructureConfig::Auto;
  std::string dest_world_structure_str = "auto";
  WorldDirectoryStructureConfig dest_world_structure =
      WorldDirectoryStructureConfig::Auto;

  std::vector<ChunkPosition> chunks;
  std::optional<i32> min_x = std::nullopt;
  std::optional<i32> max_x = std::nullopt;
  std::optional<i32> min_z = std::nullopt;
  std::optional<i32> max_z = std::nullopt;

  i32 num_threads = static_cast<i32>(std::thread::hardware_concurrency()) / 2;
  bool allow_whole_rollback = false;
  bool bulk_copy = false;
  bool dry_run = false;
  bool silent = false;
  bool verbose = false;
};

std::string dump_rollback_config(const RollbackConfig& config);

std::vector<ChunkPosition> parse_chunks(
    const std::vector<std::vector<i32>>& chunks);

}  // namespace region
