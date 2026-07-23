// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/args/arg_parser.h"

#include <cstddef>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "core/check.h"
#include "core/cli/console.h"
#include "core/cli/log_prefix.h"
#include "core/cli/style_util.h"
#include "core/core.h"

namespace app {

ArgParser::ArgParser(std::string_view program_name,
                     std::string_view version,
                     std::string_view tagline)
    : program_name_(program_name), version_(version), tagline_(tagline) {}

void ArgParser::add(ArgDef def) {
  DCHECK(!(def.required && !def.takes_value));
  defs_.push_back(std::move(def));
  matched_.push_back(false);
}

void ArgParser::add_builtin_args() {
  add({
      .long_name = "--help",
      .short_name = "-h",
      .meta = "",
      .description = "print this help message",
      .takes_value = false,
      .required = false,
      .on_match = [](std::string_view) {},
  });

  add({
      .long_name = "--version",
      .short_name = "-v",
      .meta = "",
      .description = "print version",
      .takes_value = false,
      .required = false,
      .on_match = [](std::string_view) {},
  });
  handle_builtin_args_ = true;
}

void ArgParser::provided_keys(
    const std::unordered_set<std::string>&& provided_keys) {
  provided_keys_ = provided_keys;
}

std::optional<size_t> ArgParser::find_index(std::string_view token) const {
  for (size_t i = 0; i < defs_.size(); ++i) {
    const ArgDef& def = defs_[i];
    if (def.short_name == token || def.long_name == token) {
      return i;
    }
  }
  return std::nullopt;
}

ParseResult ArgParser::parse(i32 argc, char** argv) {
  ParseResult result = ParseResult::Ok;

  if (argc == 1) {
    print_help();
    return ParseResult::PrintHelp;
  }

  for (i32 i = 1; i < argc; ++i) {
    const std::string_view token = argv[i];
    const size_t eq_pos = token.find('=');

    // split "--key=value" -> key="--key", inline_value="value"
    const std::string_view key =
        (eq_pos != std::string_view::npos) ? token.substr(0, eq_pos) : token;

    const std::optional<size_t> idx = find_index(key);

    if (!idx.has_value()) {
      std::println(stderr, "{}unknown argument: '{}'",
                   error_prefix(core::ColorMode::Auto), key);
      result = ParseResult::UnknownArgument;
      continue;
    }

    const ArgDef& def = defs_[*idx];

    if (def.takes_value) {
      std::string_view value;
      if (eq_pos != std::string_view::npos) {
        // --key=value form
        value = token.substr(eq_pos + 1);
      } else if (i + 1 < argc) {
        // --key value form
        value = argv[++i];
      } else {
        std::println(stderr, "{}argument '{}' requires a value",
                     error_prefix(core::ColorMode::Auto), key);
        result = ParseResult::MissingValue;
        continue;
      }
      if (def.on_match) {
        def.on_match(value);
      }
      matched_[*idx] = true;
    } else {
      // flag: --key=anything is an error
      if (eq_pos != std::string_view::npos) {
        std::println(stderr, "{}argument '{}' does not take a value",
                     error_prefix(core::ColorMode::Auto), key);
        continue;
      }

      if (handle_builtin_args_) {
        // short-circuit before on_match so side-effects are avoided
        if (key == "--help" || key == "-h") {
          print_help();
          return ParseResult::PrintHelp;
        } else if (key == "--version" || key == "-v") {
          print_version();
          return ParseResult::PrintVersion;
        }
      }

      if (def.on_match) {
        def.on_match({});
      }
      matched_[*idx] = true;
    }
  }

  return result;
}

bool ArgParser::validate_required() const {
  bool ok = true;
  for (size_t i = 0; i < defs_.size(); ++i) {
    if (defs_[i].required && !matched_[i]) {
      if (!provided_keys_.contains(std::string(defs_[i].long_name))) {
        std::println(stderr, "{}required argument '{}' not provided",
                     error_prefix(core::ColorMode::Auto), defs_[i].long_name);
        ok = false;
      }
    }
  }
  return ok;
}

void ArgParser::print_usage(const bool color) const {
  if (color) {
    std::print("{}{}Usage: {}{}{} {}[Options]\n\n", core::kBold,
               core::kBrightWhite, core::kGreen, program_name_, core::kReset,
               core::kMagenta);
  } else {
    std::print("Usage: {} [Options]\n\n", program_name_);
  }
}

void ArgParser::print_help() const {
  const bool color = can_use_ansi_escape_sequence(core::Stream::Stdout);

  // header
  if (color) {
    std::print("{}{}", core::kBold, core::kRed);
  }
  std::print("                     {}\n\n", program_name_);
  if (color) {
    std::print("{}", core::kReset);
  }

  // tagline
  if (!tagline_.empty()) {
    if (color) {
      std::print("{}{}", core::kBold, core::kBrightYellow);
    }
    std::print("  {}\n\n", tagline_);
    if (color) {
      std::print("{}", core::kReset);
    }
  }

  // usage line
  print_usage(color);

  // options table header
  if (color) {
    std::print("{}{}", core::kBold, core::kBrightWhite);
  }
  std::println("Options:");
  if (color) {
    std::print("{}", core::kReset);
  }

  for (const auto& d : defs_) {
    std::string left;
    left.append("  ");

    if (color) {
      left.append(core::kReset).append(core::kBold).append(core::kCyan);
    }

    if (!d.short_name.empty()) {
      left.append(d.short_name);
      left.append(", ");
    } else {
      // padding
      left.append("    ");
    }
    left.append(d.long_name);
    if (!d.meta.empty()) {
      left.push_back(' ');
      left.append(d.meta);
    }

    if (color) {
      left.append(core::kReset);
    }

    // padding
    const size_t pad = color ? 80 : 60;
    if (left.size() < pad) {
      left.append(pad - left.size(), ' ');
    }

    const std::string_view req = d.required ? " [required]" : "";
    if (color) {
      std::println("{}{}{}{}{}{}{}", left, core::kBold, core::kBrightWhite,
                   d.description, core::kDim, req, core::kReset);
    } else {
      std::println("{}{}{}", left, d.description, req);
    }
  }
  std::print("\n");
}

void ArgParser::print_version() const {
  const bool color = can_use_ansi_escape_sequence(core::Stream::Stdout);
  if (color) {
    std::println("{}{}{}{}{}  version: {}{}", core::kReset, core::kBold,
                 core::kRed, program_name_, core::kYellow, version_,
                 core::kReset);
  } else {
    std::println("{}  version: {}", program_name_, version_);
  }
}

}  // namespace app
