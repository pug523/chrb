// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <fstream>
#include <vector>

namespace core {

std::vector<bool> build_sector_map(std::fstream& file);

}
