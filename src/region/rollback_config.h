// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <optional>
#include <string>
#include <thread>

#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "region/dimension.h"
#include "region/rollback_type.h"
#include "region/world_directory_structure.h"

namespace region {

struct RollbackConfig {
  std::string src_world = "undefined";
  std::string dest_world = "undefined";
  std::string dim_str = "overworld";
  std::string type_str = "all";
  std::string color_str = "auto";
  std::string src_world_structure_str = "auto";
  std::string dest_world_structure_str = "auto";
  std::optional<i32> min_x = 0;
  std::optional<i32> max_x = 0;
  std::optional<i32> min_z = 0;
  std::optional<i32> max_z = 0;
  i32 num_threads = static_cast<i32>(std::thread::hardware_concurrency()) / 2;
  Dimension dimension = Dimension::OverWorld;
  RollbackType type = RollbackType::All;
  core::ColorMode color_mode = core::ColorMode::Auto;
  WorldDirectoryStructureConfig src_world_structure =
      WorldDirectoryStructureConfig::Auto;
  WorldDirectoryStructureConfig dest_world_structure =
      WorldDirectoryStructureConfig::Auto;
  bool bulk_copy = false;
  bool silent = false;
  bool verbose = false;
};

}  // namespace region
