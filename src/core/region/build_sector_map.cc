// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/region/build_sector_map.h"

#include <cstddef>
#include <vector>

#include "core/core.h"
#include "core/mem/mapped_file.h"

namespace core {

std::vector<bool> build_sector_map(const MappedFile& file) {
  const size_t total_sectors = file.size() / kSectorSize;
  std::vector<bool> used(total_sectors, false);

  if (total_sectors > 0) {
    used[0] = true;
  }
  if (total_sectors > 1) {
    used[1] = true;
  }

  for (i32 i = 0; i < 1024; ++i) {
    u8 buf[4];
    file.read(static_cast<size_t>(i) * 4, buf, 4);
    const i32 offset = (static_cast<i32>(buf[0]) << 16) |
                       (static_cast<i32>(buf[1]) << 8) |
                       static_cast<i32>(buf[2]);
    const u8 sectors = buf[3];
    if (offset == 0 || sectors == 0) {
      continue;
    }
    for (u8 s = 0; s < sectors; ++s) {
      const size_t index = static_cast<size_t>(offset) + s;
      if (index < total_sectors) {
        used[index] = true;
      }
    }
  }
  return used;
}

}  // namespace core
