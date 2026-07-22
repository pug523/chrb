// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/driver.h"

#include <cstddef>  // NOLINT
#include <format>
#include <print>
#include <string>
#include <utility>

#include "app/parse_args.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "core/file_util.h"
#include "region/chunk_position.h"
#include "region/rollback_config.h"
#include "region/rollback_executor.h"

namespace app {

namespace {

inline bool arg_status_contains(const ArgStatusPacked packed,
                                const ArgStatus status) {
  return (packed & static_cast<ArgStatusPacked>(status)) != 0;
}

}  // namespace

i32 rollback(i32 argc, char** argv) {
  region::RollbackConfig config;
  const ArgStatusPacked arg_result = parse_args(argc, argv, &config);

  if (config.verbose) {
    std::print("{}", region::format_rollback_config(config));
  }

  if (arg_status_contains(arg_result, ArgStatus::PrintHelp) ||
      arg_status_contains(arg_result, ArgStatus::PrintVersion)) {
    return 0;
  } else if (arg_result != static_cast<ArgStatusPacked>(ArgStatus::Success)) {
    return static_cast<i32>(arg_result);
  }

  if (!core::is_path_delimiter(config.src_world.back())) {
    config.src_world.push_back(PATH_DELIMITER);
  }
  if (!core::is_path_delimiter(config.dest_world.back())) {
    config.dest_world.push_back(PATH_DELIMITER);
  }

  {
    bool dir_exists = true;
    if (!core::is_dir(config.src_world)) {
      std::println(stderr, "{}directory not found: {}",
                   core::error_prefix(config.color_mode), config.src_world);
      dir_exists = false;
    } else if (!core::is_dir(config.dest_world)) {
      std::println(stderr, "{}directory not found: {}",
                   core::error_prefix(config.color_mode), config.dest_world);
      dir_exists = false;
    }
    if (!dir_exists) {
      return 1;
    }
  }

  region::RollbackExecutor executor;
  executor.init(&config);

  executor.start();
  executor.flush();

  const u64 successfull_region_count = executor.successfull_region_count();
  const u64 successfull_chunk_count = executor.successfull_chunk_count();
  const u64 failed_region_count = executor.failed_region_count();
  const u64 failed_chunk_count = executor.failed_chunk_count();

  if (!config.silent) {
    if (successfull_region_count > 0) {
      std::println("{}{:5} full regions processed successfully",
                   core::info_prefix(config.color_mode),
                   successfull_region_count);
    }
    if (successfull_chunk_count > 0) {
      std::println("{}{:5} chunks processed successfully",
                   core::info_prefix(config.color_mode),
                   successfull_chunk_count);
    }
  }

  i32 result = 0;
  if (failed_region_count > 0) {
    std::string failed_regions_string = "[\n";
    for (const auto& r : executor.failed_regions()) {
      failed_regions_string.append(std::format("  [{}, {}],\n", r.x, r.z));
    }
    failed_regions_string.append("]");

    std::print(R"({}{:5} full regions failed
failed_regions = {}
)",
               core::error_prefix(config.color_mode), failed_region_count,
               failed_regions_string);
    ++result;
  }
  if (failed_chunk_count > 0) {
    std::println("{}{:5} chunks failed", core::error_prefix(config.color_mode),
                 failed_chunk_count);
    ++result;
  }

  return result;
}

}  // namespace app
