// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/cli/parse_args.h"

#include <print>
#include <thread>

#include "core/args/build_parser.h"
#include "core/args/parser.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "region/dimension.h"
#include "region/rollback_config.h"
#include "region/rollback_type.h"

namespace core {

ArgStatus parse_args(i32 argc, char** argv, RollbackConfig* config) {
  ArgParser parser = build_arg_parser(config);
  const ParseResult pr = parser.parse(argc, argv);

  if (pr == ParseResult::PrintHelp) {
    return ArgStatus::PrintHelp;
  } else if (pr == ParseResult::PrintVersion) {
    return ArgStatus::PrintVersion;
  }

  const bool required_ok = parser.validate_required();
  const ArgStatus vs = validate_config(config);
  if (pr != ParseResult::Ok || !required_ok || vs != ArgStatus::Success) {
    std::println(stderr, "{}failed to parse commandline arguments",
                 error_prefix());
    parser.print_help();
    return vs != ArgStatus::Success ? vs : ArgStatus::UnknownArgument;
  }

  return ArgStatus::Success;
}

ArgStatus validate_config(RollbackConfig* config) {
  ArgStatus result = ArgStatus::Success;

  if (config->src_world.empty()) {
    std::println(stderr, "{}source world is not specified", error_prefix());
    result = ArgStatus::SourceWorldEmpty;
  }
  if (config->dest_world.empty()) {
    std::println(stderr, "{}destination world is not specified",
                 error_prefix());
    result = ArgStatus::DestinationWorldEmpty;
  }

  config->dimension = str_to_dimension(config->dim_str);
  if (config->dimension == Dimension::Unknown) {
    std::println(stderr, "{}invalid dimension: {}", error_prefix(),
                 config->dim_str);
    result = ArgStatus::InvalidDimension;
  }

  config->type = str_to_rollback_type(config->type_str);
  if (config->type == RollbackType::Unknown) {
    std::println(stderr, "{}invalid rollback type: {}", error_prefix(),
                 config->type_str);
    result = ArgStatus::InvalidRollbackType;
  }

  if (!config->min_x || !config->max_x || !config->min_z || !config->max_z) {
    std::print(stderr, "{}chunk range missing\nmin: ({},{}), max: ({},{})\n",
               error_prefix(), config->min_x.value_or(0),
               config->min_z.value_or(0), config->max_x.value_or(0),
               config->max_z.value_or(0));
    result = ArgStatus::ChunkRangeMissing;
  }

  const i32 thread_count =
      static_cast<i32>(std::thread::hardware_concurrency());
  if (config->num_threads <= 0 || thread_count < config->num_threads) {
    std::println(stderr, "{}invalid number of threads specified: {}",
                 error_prefix(), config->num_threads);
    result = ArgStatus::InvalidNumThreads;
  }

  return result;
}

}  // namespace core
