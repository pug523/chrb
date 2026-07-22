// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

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

struct RollbackConfig {
  bool config_file_enabled = false;
  std::string config_file_path = "chrb.toml";

  std::string src_world = "undefined";
  std::string dest_world = "undefined";
  std::string dim_str = "overworld";
  std::string type_str = "all";
  std::string color_str = "auto";
  std::string src_world_structure_str = "auto";
  std::string dest_world_structure_str = "auto";
  std::vector<ChunkPosition> chunks;
  std::optional<i32> min_x = std::nullopt;
  std::optional<i32> max_x = std::nullopt;
  std::optional<i32> min_z = std::nullopt;
  std::optional<i32> max_z = std::nullopt;
  i32 num_threads = static_cast<i32>(std::thread::hardware_concurrency()) / 2;
  Dimension dimension = Dimension::OverWorld;
  RollbackType type = RollbackType::All;
  core::ColorMode color_mode = core::ColorMode::Auto;
  WorldDirectoryStructureConfig src_world_structure =
      WorldDirectoryStructureConfig::Auto;
  WorldDirectoryStructureConfig dest_world_structure =
      WorldDirectoryStructureConfig::Auto;
  bool allow_whole_rollback = false;
  bool bulk_copy = false;
  bool silent = false;
  bool verbose = false;
};

std::string format_rollback_config(const RollbackConfig& config);

}  // namespace region
