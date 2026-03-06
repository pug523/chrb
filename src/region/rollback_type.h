// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/check.h"
#include "core/core.h"

namespace region {

enum class RollbackType : u8 {
  Unknown,
  Region,
  Entities,
  Poi,
  All,
};

inline RollbackType str_to_rollback_type(const std::string_view s) {
  // only supports lower case
  if (s == "region") {
    return RollbackType::Region;
  } else if (s == "entities") {
    return RollbackType::Entities;
  } else if (s == "poi") {
    return RollbackType::Poi;
  } else if (s == "all") {
    return RollbackType::All;
  } else {
    return RollbackType::Unknown;
  }
}

inline const char* type_path(RollbackType t) {
  switch (t) {
    case RollbackType::Region: return "region";
    case RollbackType::Entities: return "entities";
    case RollbackType::Poi: return "poi";
    default: dcheck(false); return "unknown";
  }
}

}  // namespace region
