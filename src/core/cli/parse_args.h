// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "core/core.h"
#include "region/rollback_config.h"

namespace core {

enum class ArgStatus : u8 {
  Success,
  PrintHelp,
  PrintVersion,
  UnknownArgument,
  SourceWorldEmpty,
  DestinationWorldEmpty,
  InvalidDimension,
  InvalidRollbackType,
  ChunkRangeMissing,
  InvalidNumThreads,
};

ArgStatus parse_args(i32 argc, char** argv, RollbackConfig* config);

// validate config after parsing and report errors
// returns ArgStatus::Success when everything is valid
ArgStatus validate_config(RollbackConfig* config);

}  // namespace core
