// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <vector>

#include "core/mem/mapped_file.h"

namespace region {

std::vector<bool> build_sector_map(const core::MappedFile& file);

}
