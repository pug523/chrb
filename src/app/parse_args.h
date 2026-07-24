// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "region/rollback_config.h"

namespace app {

using ArgStatusPacked = u8;

enum class ArgStatus : ArgStatusPacked {
  Success = 0,
  PrintHelp = 1 << 0,
  PrintVersion = 1 << 1,
  ArgParseFailed = 1 << 2,
  TomlParseFailed = 1 << 3,
  ValidationFailed = 1 << 4,
};

ArgStatusPacked parse_args(i32 argc,
                           char** argv,
                           region::RollbackConfig* config);

// validates config and returns whether config is ok or not
bool validate_config(region::RollbackConfig* config);

}  // namespace app
