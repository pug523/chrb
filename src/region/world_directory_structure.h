// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/check.h"
#include "core/core.h"
#include "core/file_util.h"
#include "region/dimension.h"
#include "region/rollback_type.h"

namespace region {

enum class WorldDirectoryStructureConfig : u8 {
  Unknown,
  Auto,
  Old,
  New,
};

enum class WorldDirectoryStructure : u8 {
  Unknown,
  Old,
  New,
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
  } else {
    return WorldDirectoryStructureConfig::Unknown;
  }
}

}  // namespace region
