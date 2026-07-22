// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/toml/toml_parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "core/check.h"
#include "core/core.h"
#include "core/file_util.h"
#include "region/rollback_config.h"
#include "toml++/toml.hpp"

namespace app {

namespace {

void parse_string(const toml::table& rc,
                  std::string_view key,
                  std::string* target,
                  std::string_view arg_key,
                  std::unordered_set<std::string>* provided_keys) {
  if (std::optional<std::string> val = rc[key].value<std::string>()) {
    *target = *val;
    if (provided_keys) {
      provided_keys->emplace(arg_key);
    }
  }
}

template <typename T>
void parse_int(const toml::table& rc,
               std::string_view key,
               T* target,
               std::string_view arg_key,
               std::unordered_set<std::string>* provided_keys) {
  if (std::optional<i64> val = rc[key].value<i64>()) {
    *target = static_cast<T>(*val);
    if (provided_keys) {
      provided_keys->emplace(arg_key);
    }
  }
}

void parse_bool(const toml::table& rc,
                std::string_view key,
                bool* target,
                std::string_view arg_key,
                std::unordered_set<std::string>* provided_keys) {
  if (std::optional<bool> val = rc[key].value<bool>()) {
    *target = *val;
    if (provided_keys) {
      provided_keys->emplace(arg_key);
    }
  }
}

}  // namespace

TomlParseStatus parse_toml_config(
    const std::string& config_path,
    region::RollbackConfig* dest,
    std::unordered_set<std::string>* provided_keys_for_arg_parser) {
  DCHECK(dest);

  if (!core::is_file(config_path)) {
    return TomlParseStatus::FileNotFound;
  }

  toml::parse_result result = toml::parse_file(config_path);

  // TODO: add error print (e.g., result.error())
  if (!result) {
    return TomlParseStatus::ParseError;
  }

  const toml::table& data = result.table();

  if (!data.contains("rollback_config")) {
    return TomlParseStatus::RollbackConfigIsNotContained;
  }

  const auto* rc = data.get_as<toml::table>("rollback_config");
  if (!rc) {
    return TomlParseStatus::RollbackConfigIsNotTable;
  }

  // String fields
  parse_string(*rc, "src_world", &dest->src_world, "src-world",
               provided_keys_for_arg_parser);
  parse_string(*rc, "dest_world", &dest->dest_world, "dest-world",
               provided_keys_for_arg_parser);
  parse_string(*rc, "dimension", &dest->dim_str, "dimension",
               provided_keys_for_arg_parser);
  parse_string(*rc, "rollback_type", &dest->type_str, "rollback-type",
               provided_keys_for_arg_parser);
  parse_string(*rc, "color", &dest->color_str, "color",
               provided_keys_for_arg_parser);
  parse_string(*rc, "src_world_structure", &dest->src_world_structure_str,
               "src-world-structure", provided_keys_for_arg_parser);
  parse_string(*rc, "dest_world_structure", &dest->dest_world_structure_str,
               "dest-world-structure", provided_keys_for_arg_parser);

  // Array field (chunks)
  if (const auto* arr = (*rc)["chunks"].as_array()) {
    std::vector<std::vector<i32>> raw_chunks;
    raw_chunks.reserve(arr->size());

    for (const auto& elem : *arr) {
      if (const auto* sub_arr = elem.as_array()) {
        std::vector<i32> chunk;
        chunk.reserve(sub_arr->size());
        for (const auto& val : *sub_arr) {
          if (auto num = val.value<i64>()) {
            chunk.push_back(static_cast<i32>(*num));
          }
        }
        raw_chunks.push_back(std::move(chunk));
      }
    }

    dest->chunks = region::parse_chunks(raw_chunks);
    if (provided_keys_for_arg_parser) {
      provided_keys_for_arg_parser->emplace("chunks");
    }
  }

  // Integer fields
  parse_int(*rc, "min_x", &dest->min_x, "min-x", provided_keys_for_arg_parser);
  parse_int(*rc, "max_x", &dest->max_x, "max-x", provided_keys_for_arg_parser);
  parse_int(*rc, "min_z", &dest->min_z, "min-z", provided_keys_for_arg_parser);
  parse_int(*rc, "max_z", &dest->max_z, "max-z", provided_keys_for_arg_parser);
  parse_int(*rc, "num_threads", &dest->num_threads, "num-threads",
            provided_keys_for_arg_parser);

  // Boolean fields
  parse_bool(*rc, "allow_whole_rollback", &dest->allow_whole_rollback,
             "allow-whole-rollback", provided_keys_for_arg_parser);
  parse_bool(*rc, "bulk_copy", &dest->bulk_copy, "bulk-copy",
             provided_keys_for_arg_parser);
  parse_bool(*rc, "dry_run", &dest->dry_run, "dry-run",
             provided_keys_for_arg_parser);
  parse_bool(*rc, "silent", &dest->silent, "silent",
             provided_keys_for_arg_parser);
  parse_bool(*rc, "verbose", &dest->verbose, "verbose",
             provided_keys_for_arg_parser);

  return TomlParseStatus::Success;
}

}  // namespace app
