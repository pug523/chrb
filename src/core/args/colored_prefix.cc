// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/args/colored_prefix.h"

#include <format>

#include "core/console.h"
#include "core/style_util.h"

namespace core {

void ColoredPrefix::init_error() {
  if (can_use_ansi_escape_sequence(Stream::Stderr)) {
    constexpr const char* kVividRed = "255;10;10m";
    formatted =
        std::format("{}{}{}error{}: ", kBold, kFgRgbPrefix, kVividRed, kReset);
  } else {
    formatted = "error: ";
  }
}

void ColoredPrefix::init_warn() {
  if (can_use_ansi_escape_sequence(Stream::Stderr)) {
    formatted = std::format("{}{}warn{}: ", kBold, kYellow, kReset);
  } else {
    formatted = "warn: ";
  }
}

}  // namespace core
