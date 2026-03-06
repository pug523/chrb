// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "core/core.h"

namespace core {

struct ArgDef {
  std::string_view long_name;   // e.g. "--src"
  std::string_view short_name;  // e.g. "-s"  (empty = no short form)
  std::string_view meta;        // e.g. "<path>"
  std::string_view description;

  // false for flags like --help, --verbose
  bool takes_value;

  bool required;

  // called when the arg is matched
  // `value` is the next token if takes_value==true, else empty
  std::function<void(std::string_view value)> on_match;
};

struct StringHash {
  using is_transparent = void;
  size_t operator()(std::string_view sv) const noexcept {
    return std::hash<std::string_view>{}(sv);
  }
  size_t operator()(const std::string& s) const noexcept {
    return std::hash<std::string_view>{}(s);
  }
};

enum class ParseResult : u8 {
  Ok,
  PrintHelp,
  PrintVersion,
  UnknownArgument,
  MissingValue,
  ValidationError,
};

class ArgParser {
 public:
  // program name shown in help
  explicit ArgParser(std::string_view program_name,
                     std::string_view version,
                     std::string_view tagline = "");

  void add(ArgDef def);

  // run the parse, returns ok on success
  // on validationerror, caller should call validate() results separately
  ParseResult parse(i32 argc, char** argv);

  bool validate_required() const;

  void print_help() const;
  void print_version() const;

 private:
  std::optional<size_t> find_index(std::string_view token) const;
  void print_usage(bool color) const;

  std::string_view program_name_;
  std::string_view version_;
  std::string_view tagline_;
  std::vector<ArgDef> defs_;
  std::unordered_map<std::string, size_t, StringHash, std::equal_to<>>
      l_to_defs_indices_;
  std::unordered_map<std::string, size_t, StringHash, std::equal_to<>>
      s_to_def_indices_;
  std::vector<bool> matched_;
};

}  // namespace core
