// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <vector>

#include "core/core.h"

namespace core {

i32 find_free_sector(const std::vector<bool>& used, u8 needed);

}
