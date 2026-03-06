// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/find_free_sector.h"

#include <cstddef>
#include <vector>

#include "core/core.h"

namespace region {

i32 find_free_sector(const std::vector<bool>& used, u8 needed) {
  size_t run = 0;
  for (size_t i = 2; i < used.size(); ++i) {
    if (!used[i]) {
      ++run;
      if (run == needed) {
        return static_cast<i32>(i - static_cast<size_t>(needed) + 1);
      }
    } else {
      run = 0;
    }
  }
  return -1;
}

}  // namespace region
