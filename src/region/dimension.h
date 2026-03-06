// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/check.h"
#include "core/core.h"

namespace region {

enum class Dimension : u8 {
  Unknown,
  OverWorld,
  Nether,
  End,
};

inline Dimension str_to_dimension(const std::string_view s) {
  // only supports lower case
  if (s == "overworld") {
    return Dimension::OverWorld;
  } else if (s == "nether") {
    return Dimension::Nether;
  } else if (s == "end") {
    return Dimension::End;
  } else {
    return Dimension::Unknown;
  }
}

inline const char* dimension_path_with_slash(Dimension d) {
  switch (d) {
    case Dimension::OverWorld: return "";
    case Dimension::Nether: return "DIM-1/";
    case Dimension::End: return "DIM1/";
    default: dcheck(false); return "unknown";
  }
}

}  // namespace region
