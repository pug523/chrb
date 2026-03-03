// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/region/location.h"

namespace core {

LocationEntry read_location(const MappedFile& file, i32 index) {
  u8 buf[4];
  file.read(static_cast<size_t>(index) * 4, buf, 4);

  return LocationEntry{
      .offset = static_cast<u32>(buf[0]) << 16 | static_cast<u32>(buf[1]) << 8 |
                static_cast<u32>(buf[2]),
      .sectors = buf[3],
  };
}

}  // namespace core
