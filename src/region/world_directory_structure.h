// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "fpag/base/numeric.h"

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

inline const char* world_dir_structure_config_to_str(
    const WorldDirectoryStructureConfig c) {
  using W = WorldDirectoryStructureConfig;
  switch (c) {
    case W::Auto: return "auto";
    case W::New: return "new";
    case W::Old: return "old";
    case W::Paper: return "paper";
    case W::Unknown: return "unknown";
  }
}

inline const char* world_dir_structure_to_str(const WorldDirectoryStructure s) {
  using W = WorldDirectoryStructure;
  switch (s) {
    case W::New: return "new";
    case W::Old: return "old";
    case W::Paper: return "paper";
    case W::Unknown: return "unknown";
  }
}

}  // namespace region
