// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string>
#include <unordered_set>

#include "region/rollback_config.h"

namespace app {

enum class TomlParseStatus : u8 {
  Success = 0,
  FileNotFound = 1 << 0,
  RollbackConfigIsNotContained = 1 << 1,
  RollbackConfigIsNotTable = 1 << 2,
};

TomlParseStatus parse_toml_config(
    const std::string& config_path,
    region::RollbackConfig* dest,
    std::unordered_set<std::string>* provided_keys_for_arg_parser);

}  // namespace app
