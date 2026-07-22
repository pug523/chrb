// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "core/core.h"
#include "region/rollback_config.h"

namespace app {

using ArgStatusPacked = u16;

enum class ArgStatus : ArgStatusPacked {
  Success = 0,
  PrintHelp = 1 << 0,
  PrintVersion = 1 << 1,
  UnknownArgument = 1 << 2,
  InvalidColorMode = 1 << 3,
  SourceWorldEmpty = 1 << 4,
  DestinationWorldEmpty = 1 << 5,
  InvalidDimension = 1 << 6,
  InvalidRollbackType = 1 << 7,
  InvalidSourceWorldStructureConfig = 1 << 8,
  InvalidDestinationWorldStructureConfig = 1 << 9,
  WholeWorldRollbackNotAllowed = 1 << 10,
  InvalidNumThreads = 1 << 11,
  InvalidToml = 1 << 12,
};

ArgStatusPacked parse_args(i32 argc,
                           char** argv,
                           region::RollbackConfig* config);

// validate config after parsing and report errors
// returns ArgStatus::Success when everything is valid
ArgStatusPacked validate_config(region::RollbackConfig* config);

}  // namespace app
