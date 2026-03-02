// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "core/core.h"

namespace core {

enum class Stream : u8 {
  Stdout,
  Stderr,
};

bool can_use_ansi_escape_sequence(Stream stream);

void register_console();

}  // namespace core
