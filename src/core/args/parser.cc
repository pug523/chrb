// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/args/parser.h"

#include <cstddef>
#include <print>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "core/check.h"
#include "core/cli/console.h"
#include "core/cli/log_prefix.h"
#include "core/cli/style_util.h"
#include "core/core.h"

namespace core {

ArgParser::ArgParser(std::string_view program_name,
                     std::string_view version,
                     std::string_view tagline)
    : program_name_(program_name), version_(version), tagline_(tagline) {}

void ArgParser::add(ArgDef def) {
  dcheck(!(def.required && !def.takes_value));
  long_name_to_defs_index_[std::string(def.long_name)] = defs_.size();
  short_name_to_defs_index_[std::string(def.short_name)] = defs_.size();
  defs_.push_back(std::move(def));
  matched_.push_back(false);
}

const ArgDef* ArgParser::find(std::string_view token) const {
  const auto& long_itr = long_name_to_defs_index_.find(std::string(token));
  if (long_itr != long_name_to_defs_index_.end()) {
    return &defs_[long_itr->second];
  }
  const auto& short_itr = short_name_to_defs_index_.find(std::string(token));
  if (short_itr != short_name_to_defs_index_.end()) {
    return &defs_[short_itr->second];
  }
  // for (const auto& d : defs_) {
  //   if (token == d.long_name ||
  //       (!d.short_name.empty() && token == d.short_name)) {
  //     return &d;
  //   }
  // }
  return nullptr;
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
    const ArgDef* def = find(token);

    if (!def) {
      std::println(stderr, "{}unknown argument: '{}'", error_prefix(), token);
      result = ParseResult::UnknownArgument;
      continue;
    } else if (def->takes_value) {
      std::string_view value;
      if (eq_pos != std::string_view::npos) {
        value = token.substr(eq_pos + 1);
      } else if (i + 1 < argc) {
        value = argv[++i];
      } else {
        std::println(stderr, "{}argument '{}' requires a value", error_prefix(),
                     token);
        continue;
      }
      def->on_match(value);
      matched_[static_cast<size_t>(def - defs_.data())] = true;
    } else if (!def->takes_value) {
      if (eq_pos != std::string_view::npos) {
        std::println(stderr, "{}argument '{}' does not take a value",
                     error_prefix(), token);
      } else {
        def->on_match({});
        // --help short-circuits
        if (token == "--help" || token == "-h") {
          print_help();
          return ParseResult::PrintHelp;
        } else if (token == "--version" || token == "-v") {
          print_version();
          return ParseResult::PrintVersion;
        }
      }

    } else {
      // unreachable
      dcheck(false);
      continue;
    }
  }

  return result;
}

bool ArgParser::validate_required() const {
  bool ok = true;
  for (size_t i = 0; i < defs_.size(); ++i) {
    if (defs_[i].required && !matched_[i]) {
      std::println(stderr, "{}required argument '{}' not provided",
                   error_prefix(), defs_[i].long_name);
      ok = false;
    }
  }
  return ok;
}

void ArgParser::print_usage(const bool color) const {
  if (color) {
    std::print("{}{}Usage: {}{}{} {}[Options]\n\n", kBold, kBrightWhite, kGreen,
               program_name_, kReset, kMagenta);
  } else {
    std::print("Usage: {} [Options]\n\n", program_name_);
  }
}

void ArgParser::print_help() const {
  const bool color = can_use_ansi_escape_sequence(Stream::Stdout);

  // header
  if (color) {
    std::print("{}{}", kBold, kRed);
  }
  std::print("                     {}\n\n", program_name_);
  if (color) {
    std::print("{}", kReset);
  }

  // tagline
  if (!tagline_.empty()) {
    if (color) {
      std::print("{}{}", kBold, kBrightYellow);
    }
    std::print("  {}\n\n", tagline_);
    if (color) {
      std::print("{}", kReset);
    }
  }

  // usage line
  print_usage(color);

  // options table
  if (color) {
    std::print("{}{}", kBold, kBrightWhite);
  }
  std::println("Options:");
  if (color) {
    std::print("{}", kReset);
  }

  for (const auto& d : defs_) {
    std::string left;
    left.append("  ");

    if (color) {
      left.append(kReset).append(kBold).append(kCyan);
    }

    if (!d.short_name.empty()) {
      left.append(d.short_name);
      left.append(", ");
    }
    left.append(d.long_name);
    if (!d.meta.empty()) {
      left.push_back(' ');
      left.append(d.meta);
    }

    if (color) {
      left.append(kReset);
    }

    // padding
    const size_t pad = color ? 64 : 40;
    if (left.size() < pad) {
      left.append(pad - left.size(), ' ');
    }
    if (color) {
      std::print("{}", kDim);
    }

    const std::string_view req = d.required ? " [required]" : " [optional]";
    if (color) {
      std::println("{}{}{}{}{}{}", left, kBold, kBrightWhite, d.description,
                   kDim, req, kReset);
    } else {
      std::println("{}{}", left, d.description);
    }
  }
  std::print("\n");
}

void ArgParser::print_version() const {
  const bool color = can_use_ansi_escape_sequence(Stream::Stdout);
  if (color) {
    std::println("{}{}{}{}{}  version: {}{}", kReset, kBold, kRed,
                 program_name_, kYellow, version_, kReset);
  } else {
    std::println("{}  version: {}", program_name_, version_);
  }
}

}  // namespace core
