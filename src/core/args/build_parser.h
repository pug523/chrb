// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "core/args/parser.h"
#include "core/region/rollback_config.h"

namespace core {

ArgParser build_arg_parser(RollbackConfig* config);

}  // namespace core
