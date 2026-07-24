// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/args/arg_parser.h"

#include <utility>

#include "fpag/arg/macro.h"

namespace app {

// fallback
#ifndef CHRB_PROJECT_VERSION
#define CHRB_PROJECT_VERSION "undefined"
#endif

namespace {

arg::Arg color_arg() {
  return arg::ArgBuilder("color")
      .help("Color mode")
      .default_value("auto")
      .choices({"auto", "always", "never"})
      .build();
}

arg::Arg config_arg() {
  return arg::ArgBuilder("config")
      .short_name('c')
      .help(
          "Use config toml file ('chrb.toml' of current directory will be "
          "used)")
      .is_flag(true)
      .build();
}

arg::Arg verbose_arg() {
  return arg::ArgBuilder("verbose")
      .short_name('V')
      .help("Enable verbose output")
      .is_flag(true)
      .build();
}

}  // namespace

arg::Parser build_preprocess_parser() {
  arg::CommandBuilder builder(CHRB_PROJECT_NAME, CHRB_PROJECT_VERSION);
  builder.builtin_enabled(false);
  builder.add_arg(color_arg());
  builder.add_arg(config_arg());
  builder.add_arg(verbose_arg());
  return arg::Parser(std::move(builder).build());
}

arg::Parser build_parser() {
  arg::CommandBuilder builder(CHRB_PROJECT_NAME, CHRB_PROJECT_VERSION);
  builder.builtin_enabled(true);
  builder.about("Chunk rollback tool for Minecraft");

  // Options requiring values
  builder.add_arg(arg::ArgBuilder("src")
                      .short_name('s')
                      .help("Source world directory")
                      .value_name("path")
                      .build());

  builder.add_arg(arg::ArgBuilder("dest")
                      .short_name('d')
                      .help("Destination world directory")
                      .value_name("path")
                      .build());

  builder.add_arg(arg::ArgBuilder("dim")
                      .short_name('D')
                      .help("Target dimension")
                      .default_value("overworld")
                      .choices({"overworld", "nether", "end"})
                      .build());

  builder.add_arg(arg::ArgBuilder("type")
                      .short_name('t')
                      .help("Rollback type")
                      .default_value("all")
                      .choices({"region", "entities", "poi", "all"})
                      .build());

  builder.add_arg(color_arg());

  builder.add_arg(arg::ArgBuilder("src-world-structure")
                      .short_name('w')
                      .help("Directory structure of source world.")
                      .default_value("auto")
                      .choices({"auto", "old", "new", "paper"})
                      .build());

  builder.add_arg(arg::ArgBuilder("dest-world-structure")
                      .short_name('W')
                      .help("Directory structure of dest world.")
                      .default_value("auto")
                      .choices({"auto", "old", "new", "paper"})
                      .build());

  builder.add_arg(arg::ArgBuilder("chunks")
                      .short_name('C')
                      .help("Chunks to rollback.")
                      .build());

  builder.add_arg(arg::ArgBuilder("min-x")
                      .short_name('x')
                      .help("Minimum chunk x coordinate constraint")
                      .value_name("integer")
                      .build());

  builder.add_arg(arg::ArgBuilder("max-x")
                      .short_name('X')
                      .help("Maximum chunk x coordinate constraint")
                      .value_name("integer")
                      .build());

  builder.add_arg(arg::ArgBuilder("min-z")
                      .short_name('z')
                      .help("Minimum chunk z coordinate constraint")
                      .value_name("integer")
                      .build());

  builder.add_arg(arg::ArgBuilder("max-z")
                      .short_name('Z')
                      .help("Maximum chunk z coordinate constraint")
                      .value_name("integer")
                      .build());

  builder.add_arg(arg::ArgBuilder("num-threads")
                      .short_name('j')
                      .help("Number of worker threads. Half of num threads on "
                            "your hardware by default")
                      .value_name("integer")
                      .build());

  // Boolean Flags
  builder.add_arg(config_arg());

  builder.add_arg(
      arg::ArgBuilder("allow-whole-rollback")
          .short_name('a')
          .help("Allow to rollback whole world when neither chunk range nor "
                "chunks are specified")
          .is_flag(true)
          .build());

  builder.add_arg(arg::ArgBuilder("bulk-copy")
                      .short_name('b')
                      .help("Use bulk copy for full region rollback")
                      .is_flag(true)
                      .build());

  builder.add_arg(
      arg::ArgBuilder("dry-run")
          .help("Only print scheduled chunks without executing rollback")
          .is_flag(true)
          .build());

  builder.add_arg(arg::ArgBuilder("silent")
                      .short_name('S')
                      .help("Disable information logging")
                      .is_flag(true)
                      .build());

  builder.add_arg(verbose_arg());

  return arg::Parser(std::move(builder).build());
}

}  // namespace app

