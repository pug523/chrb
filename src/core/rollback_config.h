// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <optional>
#include <string>

#include "core/core.h"
#include "core/dimension.h"
#include "core/rollback_type.h"

namespace core {

struct RollbackConfig {
  std::string src_world = "";
  std::string dest_world = "";
  std::string dim_str = "overworld";
  std::string type_str = "region";
  std::optional<i32> min_x = 0;
  std::optional<i32> max_x = 0;
  std::optional<i32> min_z = 0;
  std::optional<i32> max_z = 0;
  Dimension dimension = Dimension::Unknown;
  RollbackType type = RollbackType::Unknown;
  bool verbose = false;
};

}  // namespace core
