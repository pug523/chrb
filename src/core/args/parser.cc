// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/args/parser.h"

#include <print>
#include <string>
#include <utility>

#include "core/args/colored_prefix.h"
#include "core/console.h"
#include "core/style_util.h"

namespace core {

ArgParser::ArgParser(std::string_view program_name,
                     std::string_view version,
                     std::string_view tagline)
    : program_name_(program_name), version_(version), tagline_(tagline) {}

void ArgParser::add(ArgDef def) {
  dcheck(!(def.required && !def.takes_value));
  defs_.push_back(std::move(def));
  matched_.push_back(false);
}

const ArgDef* ArgParser::find(std::string_view token) const {
  for (const auto& d : defs_) {
    if (token == d.long_name ||
        (!d.short_name.empty() && token == d.short_name)) {
      return &d;
    }
  }
  return nullptr;
}

ParseResult ArgParser::parse(i32 argc, char** argv) {
  ParseResult result = ParseResult::Ok;

  for (i32 i = 1; i < argc; ++i) {
    const std::string_view token = argv[i];
    const ArgDef* def = find(token);

    if (!def) {
      ColoredPrefix ep;
      ep.init_error();
      std::println(stderr, "{}unknown argument: {}", ep.str(), token);
      result = ParseResult::UnknownArgument;
      continue;
    }

    if (!def->takes_value) {
      def->on_match({});
      // --help short-circuits
      if (token == "--help" || token == "-h") {
        print_help();
        return ParseResult::PrintHelp;
      } else if (token == "--version" || token == "-v") {
        print_version();
        return ParseResult::PrintVersion;
      }
      continue;
    }

    // needs a value token
    if (i + 1 >= argc) {
      ColoredPrefix ep;
      ep.init_error();
      std::println(stderr, "{}argument {} requires a value", ep.str(), token);
      result = ParseResult::MissingValue;
      continue;
    }
    def->on_match(argv[++i]);
    matched_[static_cast<size_t>(def - defs_.data())] = true;
  }

  return result;
}

bool ArgParser::validate_required() const {
  bool ok = true;
  for (size_t i = 0; i < defs_.size(); ++i) {
    if (defs_[i].required && !matched_[i]) {
      ColoredPrefix ep;
      ep.init_error();
      std::println(stderr, "{}required argument {} not provided", ep.str(),
                   defs_[i].long_name);
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
  std::print("\n                     {}\n\n", program_name_);
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
