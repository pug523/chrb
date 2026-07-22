// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/args/build_parser.h"

#include <charconv>
#include <cstdlib>
#include <print>
#include <string_view>
#include <system_error>

#include "app/args/parser.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "region/rollback_config.h"

namespace app {

// fallback
#ifndef CHRB_PROJECT_VERSION
#define CHRB_PROJECT_VERSION "undefined"
#endif

namespace {

bool safe_stoi(std::string_view str, i32* dest) {
  const auto [ptr, ec] =
      std::from_chars(str.data(), str.data() + str.size(), *dest);

  if (ec == std::errc()) {
    return true;
  }
  if (ec == std::errc::invalid_argument) {
    std::println(stderr, "{}invalid format: {} (expected number)",
                 error_prefix(core::ColorMode::Never), str);
    return false;
  } else if (ec == std::errc::result_out_of_range) {
    std::println(stderr, "{}out of range: {}",
                 error_prefix(core::ColorMode::Never), str);
    return false;
  } else {
    std::println(stderr, "{}unknown error: {} (expected number)",
                 error_prefix(core::ColorMode::Never), str);
    return false;
  }
}

bool parse_chunks(std::string_view str,
                  std::vector<region::ChunkPosition>* out) {
  size_t start = 0;
  while (start < str.size()) {
    size_t sep = str.find_first_of(",. ", start);
    if (sep == std::string_view::npos) {
      std::println(stderr, "{}invalid chunk format: {} (expected x.z)",
                   error_prefix(core::ColorMode::Never), str);
      return false;
    }

    std::string_view x_str = str.substr(start, sep - start);

    size_t next = str.find_first_of(", ", sep + 1);
    if (next == std::string_view::npos) {
      next = str.size();
    }

    std::string_view z_str = str.substr(sep + 1, next - (sep + 1));

    i32 x = 0;
    i32 z = 0;
    if (!safe_stoi(x_str, &x) || !safe_stoi(z_str, &z)) {
      return false;
    }

    out->push_back(region::ChunkPosition{x, z});

    start = (next == str.size()) ? next : next + 1;
    while (start < str.size() && (str[start] == ',' || str[start] == ' ')) {
      start++;
    }
  }
  return true;
}

}  // namespace

ArgParser build_arg_parser(region::RollbackConfig* config) {
  ArgParser p("chrb", CHRB_PROJECT_VERSION,
              "=-=-= chunk rollback tool for minecraft =-=-=");

  p.add({
      .long_name = "--config",
      .short_name = "-C",
      .meta = "",
      .description = "enable config toml file",
      .takes_value = false,
      .required = false,
      .on_match =
          [config](std::string_view) { config->config_file_enabled = true; },
  });

  p.add({
      .long_name = "--config-path",
      .short_name = "",
      .meta = "<path>",
      .description = "config file path (default: chrb.toml)",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) { config->config_file_path = v; },
  });

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
      .description = "target dimension (default: overworld)",
      .takes_value = true,
      .required = false,
      .on_match = [config](std::string_view v) { config->dim_str = v; },
  });

  p.add({
      .long_name = "--type",
      .short_name = "-t",
      .meta = "<region|entities|poi|all>",
      .description = "rollback type (default: all)",
      .takes_value = true,
      .required = false,
      .on_match = [config](std::string_view v) { config->type_str = v; },
  });

  p.add({
      .long_name = "--color",
      .short_name = "",
      .meta = "<auto|always|never>",
      .description = "color mode (default: auto)",
      .takes_value = true,
      .required = false,
      .on_match = [config](std::string_view v) { config->color_str = v; },
  });

  p.add({
      .long_name = "--src-world-structure",
      .short_name = "-w",
      .meta = "<auto|old|new|paper>",
      .description = "directory structure of source world. (old: DIM-1/, new: "
                     "nether/, paper: world_nether/DIM-1/) (default: auto)",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) { config->src_world_structure_str = v; },
  });

  p.add({
      .long_name = "--dest-world-structure",
      .short_name = "-W",
      .meta = "<auto|old|new|paper>",
      .description = "directory structure of dest world. (old: DIM-1/, new: "
                     "nether/, paper: world_nether/DIM-1/) (default: auto)",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) {
            config->dest_world_structure_str = v;
          },
  });

  p.add({
      .long_name = "--chunks",
      .short_name = "-C",
      .meta = "<x,z;x,z...>",
      .description =
          "comma/semicolon-separated chunk positions (e.g. 10,20;-5,15)",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) {
            if (!parse_chunks(v, &config->chunks)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--min-x",
      .short_name = "-x",
      .meta = "<n>",
      .description = "minimum chunk x coordinate constraint",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->min_x)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--max-x",
      .short_name = "-X",
      .meta = "<n>",
      .description = "maximum chunk x coordinate constraint",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->max_x)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--min-z",
      .short_name = "-z",
      .meta = "<n>",
      .description = "minimum chunk z coordinate constraint",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->min_z)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--max-z",
      .short_name = "-Z",
      .meta = "<n>",
      .description = "maximum chunk z coordinate constraint",
      .takes_value = true,
      .required = false,
      .on_match =
          [config](std::string_view v) {
            if (!safe_stoi(v, &*config->max_z)) {
              std::exit(1);
            }
          },
  });

  p.add({
      .long_name = "--num-threads",
      .short_name = "-j",
      .meta = "<n>",
      .description = "number of worker threads (default: half of num threads "
                     "on your hardware)",
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
      .long_name = "--allow-whole-rollback",
      .short_name = "-a",
      .meta = "",
      .description = "allow whole world rollback when neither chunk range nor "
                     "chunks are specified",
      .takes_value = false,
      .required = false,
      .on_match =
          [config](std::string_view) { config->allow_whole_rollback = true; },
  });

  p.add({
      .long_name = "--bulk-copy",
      .short_name = "-b",
      .meta = "",
      .description = "use bulk copy for full region rollback",
      .takes_value = false,
      .required = false,
      .on_match = [config](std::string_view) { config->bulk_copy = true; },
  });

  p.add({
      .long_name = "--silent",
      .short_name = "-S",
      .meta = "",
      .description = "disable information logging",
      .takes_value = false,
      .required = false,
      .on_match = [config](std::string_view) { config->silent = true; },
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

}  // namespace app
