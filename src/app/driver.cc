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
#include "region/rollback_config.h"
#include "region/rollback_executor.h"

i32 rollback(i32 argc, char** argv) {
  region::RollbackConfig config;
  const ArgStatus arg_result = parse_args(argc, argv, &config);
  if (arg_result == ArgStatus::PrintHelp ||
      arg_result == ArgStatus::PrintVersion) {
    return 0;
  } else if (arg_result != ArgStatus::Success) {
    return static_cast<i32>(arg_result);
  }

  if (config.src_world.back() != '/') {
    config.src_world.push_back('/');
  }

  {
    bool dir_exists = true;
    if (!core::is_dir(config.src_world)) {
      std::println(stderr, "{}directory not found: {}", core::error_prefix(),
                   config.src_world);
      dir_exists = false;
    } else if (!core::is_dir(config.dest_world)) {
      std::println(stderr, "{}directory not found: {}", core::error_prefix(),
                   config.dest_world);
      dir_exists = false;
    }
    if (!dir_exists) {
      return 1;
    }
  }

  if (config.verbose) {
    std::print(R"(
[rollback_config]

src_world = {}
dest_world = {}
dim_str = {}
type_str = {}
min_x = {}
max_x = {}
min_z = {}
max_z = {}
num_threads = {}
verbose = {}

)",
               config.src_world, config.dest_world, config.dim_str,
               config.type_str, *config.min_x, *config.max_x, *config.min_z,
               *config.max_z, config.num_threads, config.verbose);
  }

  region::RollbackExecutor executor;
  executor.init(std::move(config));

  executor.start();
  executor.flush();

  const u64 successfull_region_count = executor.successfull_region_count();
  const u64 successfull_chunk_count = executor.successfull_chunk_count();
  const u64 failed_region_count = executor.failed_region_count();
  const u64 failed_chunk_count = executor.failed_chunk_count();

  i32 result = 0;
  if (successfull_region_count > 0) {
    std::println("{}{:5} full regions processed successfully",
                 core::info_prefix(), successfull_region_count);
  }
  if (successfull_chunk_count > 0) {
    std::println("{}{:5} chunks processed successfully", core::info_prefix(),
                 successfull_chunk_count);
  }
  if (failed_region_count > 0) {
    std::string failed_regions_string = "";
    for (const auto& r : executor.failed_regions()) {
      failed_regions_string.append(std::format("r({:4}, {:4})\n", r.x, r.z));
    }
    std::print(R"({}{:5} full regions failed
[failed regions]
{}
)",
               core::error_prefix(), failed_region_count,
               failed_regions_string);
    ++result;
  }
  if (failed_chunk_count > 0) {
    std::println("{}{:5} chunks failed", core::error_prefix(),
                 failed_chunk_count);
    ++result;
  }

  return result;
}
