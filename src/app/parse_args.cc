// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/parse_args.h"

#include <optional>
#include <string>
#include <string_view>
// #include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/args/arg_parser.h"
#include "app/toml/toml_parser.h"
#include "core/file_util.h"
#include "core/logger.h"
#include "fpag/arg/error_code.h"
#include "fpag/arg/matches.h"
#include "fpag/arg/parse_error.h"
#include "region/dimension.h"
#include "region/rollback_config.h"
#include "region/rollback_type.h"
#include "region/world_directory_structure.h"

template <typename T>
struct arg::Converter<std::optional<T>> {
  static base::Result<std::optional<T>, GetError> from_string(
      std::string_view v) {
    base::Result<T, GetError> res = arg::Converter<T>::from_string(v);
    if (res.is_ok()) {
      return base::make_ok(std::make_optional(std::move(res).unwrap()));
    } else {
      return base::make_err(std::move(res).unwrap_err());
    }
  }
};

namespace app {

namespace {

bool process_partial_matches(const arg::Matches& matches,
                             region::RollbackConfig* config) {
  bool has_error = false;
  if (matches.has("color")) {
    auto res = matches.get<std::string_view>("color");
    if (res.is_ok()) {
      config->color_str = std::move(res).unwrap();
      config->color_mode = base::str_to_color_mode(config->color_str);
    } else {
      has_error = true;
    }
  }
  if (matches.has("verbose")) {
    auto res = matches.get<bool>("verbose");
    if (res.is_ok()) {
      config->verbose = std::move(res).unwrap();
    } else {
      has_error = true;
    }
  }

  return has_error;
}

void process_matches(const arg::Matches& matches,
                     region::RollbackConfig* config) {
  config->src_world =
      matches.get<std::string_view>("src").unwrap_or(config->src_world);
  config->dest_world =
      matches.get<std::string_view>("dest").unwrap_or(config->dest_world);

  config->dim_str =
      matches.get<std::string_view>("dim").unwrap_or(config->dim_str);
  config->dimension = region::str_to_dimension(config->dim_str);

  config->type_str =
      matches.get<std::string_view>("type").unwrap_or(config->type_str);
  config->type = region::str_to_rollback_type(config->type_str);

  config->src_world_structure_str =
      matches.get<std::string_view>("src-world-structure")
          .unwrap_or(config->src_world_structure_str);
  config->src_world_structure =
      region::str_to_world_dir_structure(config->src_world_structure_str);

  config->dest_world_structure_str =
      matches.get<std::string_view>("dest-world-structure")
          .unwrap_or(config->dest_world_structure_str);
  config->dest_world_structure =
      region::str_to_world_dir_structure(config->dest_world_structure_str);

  const std::vector<std::string_view> raw_chunks =
      matches.get_all<std::string_view>("chunks").unwrap_or({});
  config->chunks = region::parse_chunks(raw_chunks);
  config->min_x =
      matches.get<std::optional<i32>>("min-x").unwrap_or(config->min_x);
  config->max_x =
      matches.get<std::optional<i32>>("max-x").unwrap_or(config->max_x);
  config->min_z =
      matches.get<std::optional<i32>>("min-z").unwrap_or(config->min_z);
  config->max_z =
      matches.get<std::optional<i32>>("max-z").unwrap_or(config->max_z);

  config->num_threads =
      matches.get<i32>("num-threads").unwrap_or(config->num_threads);
  config->allow_whole_rollback =
      matches.get<bool>("allow-whole-rollback").unwrap_or(false);
  config->bulk_copy = matches.get<bool>("bulk-copy").unwrap_or(false);
  config->dry_run = matches.get<bool>("dry-run").unwrap_or(false);
  config->silent = matches.get<bool>("silent").unwrap_or(false);
}

ArgStatusPacked process(const arg::Matches& matches,
                        region::RollbackConfig* config) {
  process_matches(matches, config);
  const bool vs = validate_config(config);
  return static_cast<ArgStatusPacked>(vs ? ArgStatus::Success
                                         : ArgStatus::ValidationFailed);
}

}  // namespace

ArgStatusPacked parse_args(i32 argc,
                           char** argv,
                           region::RollbackConfig* config) {
  arg::Parser preprocess_parser = build_preprocess_parser();
  arg::Parser full_parser = build_parser();

  arg::Matches partial_matches;
  std::vector<std::string_view> unparsed;
  preprocess_parser.parse_partial(argc, argv, &partial_matches, &unparsed);

  bool enable_toml = false;
  bool has_error = false;
  if (partial_matches.has("config")) {
    auto res = partial_matches.get<bool>("config");
    if (res.is_ok()) {
      enable_toml = std::move(res).unwrap();
    } else {
      has_error = true;
    }
  }
  has_error |= process_partial_matches(partial_matches, config);
  const base::ColorStyle color_style =
      base::console_color_style(base::Stream::Stdout, config->color_mode);
  core::init_logger(color_style);

  if (has_error) {
    core::logger.error("failed to parse arguments\n{}",
                       preprocess_parser.error_message());
    return static_cast<ArgStatusPacked>(ArgStatus::ArgParseFailed);
  }

  std::unordered_set<std::string> provided_options_from_toml;
  if (enable_toml) {
    const std::string kDefaultConfigFileName = "chrb.toml";
    const TomlParseStatus toml_status = parse_toml_config(
        kDefaultConfigFileName, config, &provided_options_from_toml);
    // if (config->verbose) {
    //   core::logger.info("loaded from toml: \n{}",
    //                     region::dump_rollback_config(*config));
    // }

    if (toml_status != TomlParseStatus::Success) {
      core::logger.error("failed to parse toml config ({})\n\n{}",
                         static_cast<u8>(toml_status),
                         full_parser.help_message());
      return static_cast<ArgStatusPacked>(ArgStatus::TomlParseFailed);
    }
  }

  arg::ParseResult<arg::Matches> pr = full_parser.try_parse(argc, argv);
  if (pr.is_ok()) {
    return process(std::move(pr).unwrap(), config);
  } else if (pr.is_err()) {
    core::logger.error("failed to parse arguments\n{}",
                       full_parser.error_message());
    return static_cast<ArgStatusPacked>(ArgStatus::ArgParseFailed);
  } else if (pr.is_help()) {
    core::logger.info("\n{}", full_parser.help_message());
    return static_cast<ArgStatusPacked>(ArgStatus::PrintHelp);
  } else if (pr.is_version()) {
    // TODO: Use version formatter when it fixes
    // core::logger.info("{}", full_parser.version_message());
    core::logger.info("{} version {}", CHRB_PROJECT_NAME, CHRB_PROJECT_VERSION);
    return static_cast<ArgStatusPacked>(ArgStatus::PrintVersion);
  }
  FPAG_UNREACHABLE();
}

bool validate_config(region::RollbackConfig* config) {
  bool is_ok = true;

  config->color_mode = base::str_to_color_mode(config->color_str);
  if (config->color_mode == base::ColorMode::Unknown) {
    core::logger.error("invalid color mode: ", config->color_str);
    is_ok = false;
    config->color_mode = base::ColorMode::Auto;
  }

  if (config->src_world.empty()) {
    core::logger.error("source world is not specified");
    is_ok = false;
  } else if (core::is_dir(config->src_world)) {
    core::logger.error("source world directory does not exist");
    is_ok = false;
  }
  if (config->dest_world.empty()) {
    core::logger.error("destination world is not specified");
    is_ok = false;
  } else if (core::is_dir(config->dest_world)) {
    core::logger.error("destination world directory does not exist");
    is_ok = false;
  }

  config->dimension = region::str_to_dimension(config->dim_str);
  if (config->dimension == region::Dimension::Unknown) {
    core::logger.error("invalid dimension: {}", config->dim_str);
    is_ok = false;
  }

  config->type = region::str_to_rollback_type(config->type_str);
  if (config->type == region::RollbackType::Unknown) {
    core::logger.error("invalid rollback type: {}", config->type_str);
    is_ok = false;
  }

  config->src_world_structure =
      region::str_to_world_dir_structure(config->src_world_structure_str);
  if (config->src_world_structure ==
      region::WorldDirectoryStructureConfig::Unknown) {
    core::logger.error("invalid src world structure specified: {}",
                       config->src_world_structure_str);
    is_ok = false;
  }

  config->dest_world_structure =
      region::str_to_world_dir_structure(config->dest_world_structure_str);
  if (config->dest_world_structure ==
      region::WorldDirectoryStructureConfig::Unknown) {
    core::logger.error("invalid dest world structure specified: {}",
                       config->dest_world_structure_str);
    is_ok = false;
  }

  const bool has_range_constraint =
      config->min_x || config->max_x || config->min_z || config->max_z;
  const bool has_chunks = !config->chunks.empty();
  if (!config->allow_whole_rollback && !(has_range_constraint || has_chunks)) {
    core::logger.error(
        "whole world rollback is not allowed. pass '--allow-whole-rollback' to "
        "execute no range specified rollback.");
    is_ok = false;
  }

  // const i32 thread_count =
  //     static_cast<i32>(std::thread::hardware_concurrency());
  // if (config->num_threads <= 0 || thread_count < config->num_threads) {
  if (config->num_threads <= 0) {
    core::logger.error("invalid number of threads specified: {}",
                       config->num_threads);
    is_ok = false;
  }

  if (!is_ok) {
    core::logger.info("For more information, run 'chrb --help'");
  }

  return is_ok;
}

}  // namespace app
