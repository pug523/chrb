// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/check.h"
#include "fpag/base/numeric.h"

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

inline const char* dimension_to_str(Dimension d) {
  switch (d) {
    case Dimension::OverWorld: return "overworld";
    case Dimension::Nether: return "nether";
    case Dimension::End: return "end";
    default: DCHECK(false); return "unknown";
  }
}

inline const char* dimension_path_with_slash_old(Dimension d) {
  switch (d) {
    case Dimension::OverWorld: return "";
    case Dimension::Nether: return "DIM-1/";
    case Dimension::End: return "DIM1/";
    default: DCHECK(false); return "unknown";
  }
}

inline const char* dimension_path_with_slash_new(Dimension d) {
  switch (d) {
    case Dimension::OverWorld: return "dimensions/minecraft/overworld/";
    case Dimension::Nether: return "dimensions/minecraft/the_nether/";
    case Dimension::End: return "dimensions/minecraft/the_end/";
    default: DCHECK(false); return "unknown";
  }
}

inline const char* dimension_path_with_slash_paper(Dimension d) {
  switch (d) {
    case Dimension::OverWorld: return "world/";
    case Dimension::Nether: return "world_nether/DIM-1/";
    case Dimension::End: return "world_the_end/DIM1/";
    default: DCHECK(false); return "unknown";
  }
}

}  // namespace region
