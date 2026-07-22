// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/parse_args.h"

#include <print>
#include <thread>
#include <unordered_set>

#include "app/args/arg_parser.h"
#include "app/args/build_parser.h"
#include "app/toml/toml_parser.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "region/dimension.h"
#include "region/rollback_config.h"
#include "region/rollback_type.h"
#include "region/world_directory_structure.h"

namespace app {

namespace {

TomlParseStatus load_toml_if_needed(
    i32 argc,
    char** argv,
    region::RollbackConfig* config,
    std::unordered_set<std::string>* provided_keys_for_arg_parser) {
  bool config_file_enabled = false;
  bool config_file_specified = false;
  bool path_next = false;
  std::string_view config_path = "chrb.toml";
  for (i32 i = 0; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (path_next) {
      config_path = arg;
      config_file_specified = true;
      path_next = false;
    } else if (arg == "-c" || arg == "--config") {
      config_file_enabled = true;
    } else if (arg == "--config-path") {
      path_next = true;
    }
  }

  if (config_file_enabled) {
    return parse_toml_config(std::string(config_path), config,
                             provided_keys_for_arg_parser);
  } else if (config_file_specified) {
    std::println(
        stderr,
        R"({}config file is specified, but config ("--config" or "-c") is not enabled.)",
        core::warn_prefix(config->color_mode));
  }
  return TomlParseStatus::Success;
}

}  // namespace

ArgStatusPacked parse_args(i32 argc,
                           char** argv,
                           region::RollbackConfig* config) {
  ArgParser parser = build_arg_parser(config);
  std::unordered_set<std::string> provided_keys_for_arg_parser;
  const TomlParseStatus toml_status =
      load_toml_if_needed(argc, argv, config, &provided_keys_for_arg_parser);
  parser.provided_keys(std::move(provided_keys_for_arg_parser));

  if (config->verbose) {
    std::print("loaded from toml: \n{}", region::dump_rollback_config(*config));
  }

  if (toml_status != TomlParseStatus::Success) {
    std::print(stderr, "{}failed to parse toml config\n\n",
               core::error_prefix(config->color_mode));
    parser.print_help();
    return static_cast<ArgStatusPacked>(ArgStatus::InvalidToml);
  }

  const ParseResult pr = parser.parse(argc, argv);

  if (pr == ParseResult::PrintHelp) {
    return static_cast<ArgStatusPacked>(ArgStatus::PrintHelp);
  } else if (pr == ParseResult::PrintVersion) {
    return static_cast<ArgStatusPacked>(ArgStatus::PrintVersion);
  }

  const ArgStatusPacked vs = validate_config(config);
  const bool required_ok = parser.validate_required();
  if (pr != ParseResult::Ok || !required_ok ||
      vs != static_cast<ArgStatusPacked>(ArgStatus::Success)) {
    std::print(stderr, "{}failed to parse commandline arguments\n\n",
               core::error_prefix(config->color_mode));
    parser.print_help();
    return vs | static_cast<ArgStatusPacked>(ArgStatus::UnknownArgument);
  }

  return static_cast<ArgStatusPacked>(ArgStatus::Success);
}

ArgStatusPacked validate_config(region::RollbackConfig* config) {
  ArgStatusPacked result = static_cast<ArgStatusPacked>(ArgStatus::Success);

  config->color_mode = core::str_to_color_mode(config->color_str);
  if (config->color_mode == core::ColorMode::Unknown) {
    std::println(stderr, "{}invalid color mode: {}",
                 core::error_prefix(core::ColorMode::Never), config->color_str);
    result |= static_cast<ArgStatusPacked>(ArgStatus::InvalidColorMode);
    config->color_mode = core::ColorMode::Auto;
  }

  if (config->src_world.empty()) {
    std::println(stderr, "{}source world is not specified",
                 core::error_prefix(config->color_mode));
    result |= static_cast<ArgStatusPacked>(ArgStatus::SourceWorldEmpty);
  }
  if (config->dest_world.empty()) {
    std::println(stderr, "{}destination world is not specified",
                 core::error_prefix(config->color_mode));
    result |= static_cast<ArgStatusPacked>(ArgStatus::DestinationWorldEmpty);
  }

  config->dimension = region::str_to_dimension(config->dim_str);
  if (config->dimension == region::Dimension::Unknown) {
    std::println(stderr, "{}invalid dimension: {}",
                 core::error_prefix(config->color_mode), config->dim_str);
    result |= static_cast<ArgStatusPacked>(ArgStatus::InvalidDimension);
  }

  config->type = region::str_to_rollback_type(config->type_str);
  if (config->type == region::RollbackType::Unknown) {
    std::println(stderr, "{}invalid rollback type: {}",
                 core::error_prefix(config->color_mode), config->type_str);
    result |= static_cast<ArgStatusPacked>(ArgStatus::InvalidRollbackType);
  }

  config->src_world_structure =
      region::str_to_world_dir_structure(config->src_world_structure_str);
  if (config->src_world_structure ==
      region::WorldDirectoryStructureConfig::Unknown) {
    std::println(stderr, "{}invalid src world structure specified: {}",
                 core::error_prefix(config->color_mode),
                 config->src_world_structure_str);
    result |= static_cast<ArgStatusPacked>(
        ArgStatus::InvalidSourceWorldStructureConfig);
  }

  config->dest_world_structure =
      region::str_to_world_dir_structure(config->dest_world_structure_str);
  if (config->dest_world_structure ==
      region::WorldDirectoryStructureConfig::Unknown) {
    std::println(stderr, "{}invalid dest world structure specified: {}",
                 core::error_prefix(config->color_mode),
                 config->dest_world_structure_str);
    result |= static_cast<ArgStatusPacked>(
        ArgStatus::InvalidDestinationWorldStructureConfig);
  }

  const bool has_range_constraint =
      config->min_x || config->max_x || config->min_z || config->max_z;
  const bool has_chunks = !config->chunks.empty();
  if (!config->allow_whole_rollback && !(has_range_constraint || has_chunks)) {
    std::print(stderr, "{}whole world rollback is not allowed\n",
               core::error_prefix(config->color_mode));
    result |=
        static_cast<ArgStatusPacked>(ArgStatus::WholeWorldRollbackNotAllowed);
  }

  const i32 thread_count =
      static_cast<i32>(std::thread::hardware_concurrency());
  if (config->num_threads <= 0 || thread_count < config->num_threads) {
    std::println(stderr, "{}invalid number of threads specified: {}",
                 core::error_prefix(config->color_mode), config->num_threads);
    result |= static_cast<ArgStatusPacked>(ArgStatus::InvalidNumThreads);
  }

  return result;
}

}  // namespace app
