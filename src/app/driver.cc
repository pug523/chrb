// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/driver.h"

#include <cstddef>  // NOLINT
#include <format>
#include <string>
#include <utility>

#include "app/parse_args.h"
#include "core/file_util.h"
#include "core/logger.h"
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
    core::logger.info("loaded config: {}",
                      region::dump_rollback_config(config));
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
      core::logger.error("src directory not found: {}", config.src_world);
      dir_exists = false;
    } else if (!core::is_dir(config.dest_world)) {
      core::logger.error("dest directory not found: {}", config.dest_world);
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

  const u64 successful_region_count = executor.successful_region_count();
  const u64 successful_chunk_count = executor.successful_chunk_count();
  const u64 failed_region_count = executor.failed_region_count();
  const u64 failed_chunk_count = executor.failed_chunk_count();

  if (!config.silent) {
    if (successful_region_count > 0) {
      core::logger.info("{:5} full regions processed successfully",
                        successful_region_count);
    }
    if (successful_chunk_count > 0) {
      core::logger.info("{:5} chunks processed successfully",
                        successful_chunk_count);
    }
  }

  i32 result = 0;
  if (failed_region_count > 0) {
    std::string failed_regions_string = "[\n";
    for (const auto& r : executor.failed_regions()) {
      failed_regions_string.append(std::format("  [{}, {}],\n", r.x, r.z));
    }
    failed_regions_string.append("]");

    core::logger.error("{:5} full regions failed\nfailed_regions = {}",
                       failed_region_count, failed_regions_string);
    ++result;
  }
  if (failed_chunk_count > 0) {
    core::logger.error("{:5} chunks failed", failed_chunk_count);
    ++result;
  }

  return result;
}

}  // namespace app
