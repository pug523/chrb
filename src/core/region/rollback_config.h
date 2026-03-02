// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <optional>
#include <string>
#include <thread>

#include "core/core.h"
#include "core/region/dimension.h"
#include "core/region/rollback_type.h"

namespace core {

struct RollbackConfig {
  std::string src_world = "";
  std::string dest_world = "";
  std::string dim_str = "overworld";
  std::string type_str = "all";
  std::optional<i32> min_x = 0;
  std::optional<i32> max_x = 0;
  std::optional<i32> min_z = 0;
  std::optional<i32> max_z = 0;
  i32 num_threads = static_cast<i32>(std::thread::hardware_concurrency()) / 2;
  Dimension dimension = Dimension::Unknown;
  RollbackType type = RollbackType::Unknown;
  bool verbose = false;
};

}  // namespace core
