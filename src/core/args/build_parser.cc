// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/args/build_parser.h"

#include <print>
#include <string>

#include "core/args/colored_prefix.h"
#include "core/console.h"
#include "core/dimension.h"
#include "core/rollback_type.h"
#include "core/style_util.h"

namespace core {

// fallback
#ifndef PROJECT_VERSION
#define PROJECT_VERSION "0.0.0"
#endif

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
      .required = true,
      .on_match = [config](std::string_view v) { config->dim_str = v; },
  });

  p.add({
      .long_name = "--type",
      .short_name = "-t",
      .meta = "<region|entities|poi>",
      .description = "rollback type",
      .takes_value = true,
      .required = true,
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
            config->min_x = std::stoi(std::string{v});
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
            config->max_x = std::stoi(std::string{v});
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
            config->min_z = std::stoi(std::string{v});
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
            config->max_z = std::stoi(std::string{v});
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
