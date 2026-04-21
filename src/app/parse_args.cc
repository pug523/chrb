// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/parse_args.h"

#include <print>
#include <thread>

#include "app/args/build_parser.h"
#include "app/args/parser.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "region/dimension.h"
#include "region/rollback_config.h"
#include "region/rollback_type.h"
#include "region/world_directory_structure.h"

namespace app {

ArgStatus parse_args(i32 argc, char** argv, region::RollbackConfig* config) {
  ArgParser parser = build_arg_parser(config);
  const ParseResult pr = parser.parse(argc, argv);

  if (pr == ParseResult::PrintHelp) {
    return ArgStatus::PrintHelp;
  } else if (pr == ParseResult::PrintVersion) {
    return ArgStatus::PrintVersion;
  }

  const ArgStatus vs = validate_config(config);
  const bool required_ok = parser.validate_required();
  if (pr != ParseResult::Ok || !required_ok || vs != ArgStatus::Success) {
    std::println(stderr, "{}failed to parse commandline arguments",
                 core::error_prefix(config->color_mode));
    parser.print_help();
    return vs != ArgStatus::Success ? vs : ArgStatus::UnknownArgument;
  }

  return ArgStatus::Success;
}

ArgStatus validate_config(region::RollbackConfig* config) {
  ArgStatus result = ArgStatus::Success;

  config->color_mode = core::str_to_color_mode(config->color_str);
  if (config->color_mode == core::ColorMode::Unknown) {
    std::println(stderr, "{}invalid color mode: {}",
                 core::error_prefix(core::ColorMode::Never), config->color_str);
    result = ArgStatus::InvalidColorMode;
    config->color_mode = core::ColorMode::Auto;
  }

  if (config->src_world.empty()) {
    std::println(stderr, "{}source world is not specified",
                 core::error_prefix(config->color_mode));
    result = ArgStatus::SourceWorldEmpty;
  }
  if (config->dest_world.empty()) {
    std::println(stderr, "{}destination world is not specified",
                 core::error_prefix(config->color_mode));
    result = ArgStatus::DestinationWorldEmpty;
  }

  config->dimension = region::str_to_dimension(config->dim_str);
  if (config->dimension == region::Dimension::Unknown) {
    std::println(stderr, "{}invalid dimension: {}",
                 core::error_prefix(config->color_mode), config->dim_str);
    result = ArgStatus::InvalidDimension;
  }

  config->type = region::str_to_rollback_type(config->type_str);
  if (config->type == region::RollbackType::Unknown) {
    std::println(stderr, "{}invalid rollback type: {}",
                 core::error_prefix(config->color_mode), config->type_str);
    result = ArgStatus::InvalidRollbackType;
  }

  config->src_world_structure =
      region::str_to_world_dir_structure(config->src_world_structure_str);
  if (config->src_world_structure ==
      region::WorldDirectoryStructureConfig::Unknown) {
    std::println(stderr, "{}invalid src world structure specified: {}",
                 core::error_prefix(config->color_mode),
                 config->src_world_structure_str);
    result = ArgStatus::InvalidSourceWorldStructureConfig;
  }

  config->dest_world_structure =
      region::str_to_world_dir_structure(config->dest_world_structure_str);
  if (config->dest_world_structure ==
      region::WorldDirectoryStructureConfig::Unknown) {
    std::println(stderr, "{}invalid dest world structure specified: {}",
                 core::error_prefix(config->color_mode),
                 config->dest_world_structure_str);
    result = ArgStatus::InvalidDestinationWorldStructureConfig;
  }

  if (!config->min_x || !config->max_x || !config->min_z || !config->max_z) {
    std::print(stderr, "{}chunk range missing\nmin: ({},{}), max: ({},{})\n",
               core::error_prefix(config->color_mode),
               config->min_x.value_or(0), config->min_z.value_or(0),
               config->max_x.value_or(0), config->max_z.value_or(0));
    result = ArgStatus::ChunkRangeMissing;
  }

  const i32 thread_count =
      static_cast<i32>(std::thread::hardware_concurrency());
  if (config->num_threads <= 0 || thread_count < config->num_threads) {
    std::println(stderr, "{}invalid number of threads specified: {}",
                 core::error_prefix(config->color_mode), config->num_threads);
    result = ArgStatus::InvalidNumThreads;
  }

  return result;
}

}  // namespace app
