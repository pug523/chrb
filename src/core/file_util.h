// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "core/core.h"

namespace core {

bool is_dir(const std::string_view path);
bool is_file(const std::string_view path);
bool create_file(const std::string_view path);
bool copy_file(const std::string_view from, const std::string_view to);
bool rename_file(const std::string_view from, const std::string_view to);
i64 count_files(const std::string_view dir);
std::vector<std::string> list_files(const std::string_view dir);

#ifdef IS_PLAT_LINUX
std::vector<u64> async_copy_files(const std::vector<std::string>& srcs,
                                  const std::vector<std::string>& dests,
                                  u32 queue_depth);
#endif

#ifdef IS_PLAT_WINDOWS
#define PATH_DELIMITER '\\'
#else
#define PATH_DELIMITER '/'
#endif

}  // namespace core
