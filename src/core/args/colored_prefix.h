// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string>
#include <string_view>

namespace core {

class ColoredPrefix {
 public:
  ColoredPrefix() = default;
  ~ColoredPrefix() = default;

  ColoredPrefix(const ColoredPrefix&) = delete;
  ColoredPrefix& operator=(const ColoredPrefix&) = delete;

  ColoredPrefix(ColoredPrefix&&) noexcept = default;
  ColoredPrefix& operator=(ColoredPrefix&&) noexcept = default;

  void init_error();
  void init_warn();
  std::string_view str() const { return formatted; }

 private:
  std::string formatted;
};

}  // namespace core
