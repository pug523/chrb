// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/args/build_parser.h"

#include <charconv>
#include <cstdlib>
#include <print>
#include <string_view>
#include <system_error>

#include "core/args/parser.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "region/rollback_config.h"

namespace core {

// fallback
#ifndef PROJECT_VERSION
#define PROJECT_VERSION "0.0.0"
#endif

namespace {

bool safe_stoi(std::string_view str, i32* dest) {
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), *dest);

  if (ec == std::errc()) {
    return true;
  }
  if (ec == std::errc::invalid_argument) {
    std::println(stderr, "{}invalid format: {} (expected number)",
                 error_prefix(), str);
    return false;
  } else if (ec == std::errc::result_out_of_range) {
    std::println(stderr, "{}out of range: {}", error_prefix(), str);
    return false;
  } else {
    std::println(stderr, "{}unknown error: {} (expected number)",
                 error_prefix(), str);
    return false;
  }
}

}  // namespace

ArgParser build_arg_parser(RollbackConfig* config) {
  ArgParser p("chrb", PROJECT_VERSION,
              "=-=-= chunk rollback tool for minecraft =-=-=");

  p.add({
      .long_name = "--src",
      .short_name = "-s",
      .meta = "<path>",
      .description = "source world directory",
      .takes_value = true,
      .required = true,
      .on_match = [config](std::string_view v) { config->src_world = v; },
  });

  p.add({
      .long_name = "--dest",
      .short_name = "-d",
      .meta = "<path>",
      .description = "destination world directory",
      .takes_value = true,
      .required = true,
      .on_match = [config](std::string_view v) { config->dest_world = v; },
  });

  p.add({
      .long_name = "--dim",
      .short_name = "-D",
      .meta = "<overworld|nether|end>",
      .description = "target dimension",
      .takes_value = true,
      .required = false,
      .on_match = [config](std::string_view v) { config->dim_str = v; },
  });

  p.add({
      .long_name = "--type",
      .short_name = "-t",
      .meta = "<region|entities|poi|all>",
      .description = "rollback type",
      .takes_value = true,
      .required = false,
      .on_match = [config](std::string_view v) { config->type_str = v; },
  });

  p.add({
      .long_name = "--min_x",
      .short_name = "-x",
      .meta = "<n>",
      .description = "minimum chunk x coordinate",
      .takes_value = true,
      .required = true,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->min_x)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--max_x",
      .short_name = "-X",
      .meta = "<n>",
      .description = "maximum chunk x coordinate",
      .takes_value = true,
      .required = true,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->max_x)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--min_z",
      .short_name = "-z",
      .meta = "<n>",
      .description = "minimum chunk z coordinate",
      .takes_value = true,
      .required = true,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->min_z)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--max_z",
      .short_name = "-Z",
      .meta = "<n>",
      .description = "maximum chunk z coordinate",
      .takes_value = true,
      .required = true,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->max_z)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--num_threads",
      .short_name = "-j",
      .meta = "<n>",
      .description = "number of worker threads",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &config->num_threads)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--verbose",
      .short_name = "-V",
      .meta = "",
      .description = "enable verbose output",
      .takes_value = false,
      .required = false,
      .on_match = [config](std::string_view) { config->verbose = true; },
  });

  p.add({
      .long_name = "--help",
      .short_name = "-h",
      .meta = "",
      .description = "print this help message",
      .takes_value = false,
      .required = false,
      // handled inside ArgParser
      .on_match = [](std::string_view) {},
  });

  p.add({
      .long_name = "--version",
      .short_name = "-v",
      .meta = "",
      .description = "print version",
      .takes_value = false,
      .required = false,
      // handled inside ArgParser
      .on_match = [](std::string_view) {},
  });

  return p;
}

}  // namespace core
