// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/build_sector_map.h"

#include <fstream>
#include <vector>

#include "core/core.h"

namespace core {

std::vector<bool> build_sector_map(std::fstream& file) {
  file.seekg(0, std::ios::end);
  const std::streamoff file_size = file.tellg();
  const std::streamoff sector_count =
      (file_size + kSectorSize - 1) / kSectorSize;

  std::vector<bool> used(static_cast<size_t>(sector_count), false);
  // header
  used[0] = used[1] = true;

  file.seekg(0);
  for (i32 i = 0; i < 1024; ++i) {
    u8 buf[4];
    file.read(reinterpret_cast<char*>(buf), 4);

    const u32 offset = static_cast<u32>((buf[0]) << 16) |
                       static_cast<u32>((buf[1]) << 8) |
                       static_cast<u32>(buf[2]);
    const u8 sectors = buf[3];

    if (offset == 0) {
      continue;
    }

    for (u8 s = 0; s < sectors; ++s) {
      if (offset + s < used.size()) {
        used[offset + s] = true;
      }
    }
  }
  return used;
}

}  // namespace core
