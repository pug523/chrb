// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/core.h"

namespace region {

enum class WorldDirectoryStructureConfig : u8 {
  Unknown,
  Auto,
  Old,
  New,
  Paper,
};

enum class WorldDirectoryStructure : u8 {
  Unknown,
  Old,
  New,
  Paper,
};

inline WorldDirectoryStructureConfig str_to_world_dir_structure(
    const std::string_view s) {
  // only supports lower case
  if (s == "auto") {
    return WorldDirectoryStructureConfig::Auto;
  } else if (s == "old") {
    return WorldDirectoryStructureConfig::Old;
  } else if (s == "new") {
    return WorldDirectoryStructureConfig::New;
  } else if (s == "paper") {
    return WorldDirectoryStructureConfig::Paper;
  } else {
    return WorldDirectoryStructureConfig::Unknown;
  }
}

}  // namespace region
