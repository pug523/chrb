// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/region/location.h"

#include <fstream>

namespace core {

LocationEntry read_location(std::fstream& file, i32 index) {
  file.seekg(index * 4);
  u8 buf[4];
  file.read(reinterpret_cast<char*>(buf), 4);

  return LocationEntry{
      .offset = static_cast<u32>(buf[0]) << 16 | static_cast<u32>(buf[1]) << 8 |
                static_cast<u32>(buf[2]),
      .sectors = buf[3],
  };
}

}  // namespace core
