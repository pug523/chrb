// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

namespace core {

bool is_dir(const std::string_view path);
bool is_file(const std::string_view path);

}  // namespace core
