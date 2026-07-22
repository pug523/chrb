// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/toml/toml_parser.h"

#include <string>
#include <string_view>
#include <toml.hpp>
#include <unordered_set>
#include <vector>

#include "app/driver.h"
#include "core/check.h"
#include "core/cli/console.h"
#include "core/file_util.h"
#include "region/chunk_position.h"

namespace app {

TomlParseStatus parse_toml_config(
    const std::string& config_path,
    region::RollbackConfig* dest,
    std::unordered_set<std::string>* provided_keys_for_arg_parser) {
  DCHECK(dest);

  if (!core::is_file(config_path)) {
    return TomlParseStatus::FileNotFound;
  }

  // we use toml v1.1.0
  auto data = toml::parse(config_path, toml::spec::v(1, 1, 0));

  if (!data.contains("rollback_config")) {
    return TomlParseStatus::RollbackConfigIsNotContained;
  }
  if (!data.at("rollback_config").is_table()) {
    return TomlParseStatus::RollbackConfigIsNotTable;
  }

  auto rc = data.at("rollback_config");

  if (rc.contains("src_world") && rc.at("src_world").is_string()) {
    dest->src_world = rc["src_world"].as_string();
    provided_keys_for_arg_parser->emplace("src-world");
  }
  if (rc.contains("dest_world") && rc.at("dest_world").is_string()) {
    dest->dest_world = rc["dest_world"].as_string();
    provided_keys_for_arg_parser->emplace("dest-world");
  }
  if (rc.contains("dimension") && rc.at("dimension").is_string()) {
    dest->dim_str = rc["dimension"].as_string();
    provided_keys_for_arg_parser->emplace("dimension");
  }
  if (rc.contains("rollback_type") && rc.at("rollback_type").is_string()) {
    dest->type_str = rc["rollback_type"].as_string();
    provided_keys_for_arg_parser->emplace("rollback-type");
  }
  if (rc.contains("color") && rc.at("color").is_string()) {
    dest->color_str = rc["color"].as_string();
    provided_keys_for_arg_parser->emplace("color");
  }
  if (rc.contains("src_world_structure") &&
      rc.at("src_world_structure").is_string()) {
    dest->src_world_structure_str = rc["src_world_structure"].as_string();
    provided_keys_for_arg_parser->emplace("src-world-structure");
  }
  if (rc.contains("dest_world_structure") &&
      rc.at("dest_world_structure").is_string()) {
    dest->src_world_structure_str = rc["dest_world_structure"].as_string();
    provided_keys_for_arg_parser->emplace("dest-world-structure");
  }
  if (rc.contains("chunks") && rc.at("chunks").is_array()) {
    const auto raw_chunks =
        toml::find<std::vector<std::vector<i32>>>(rc, "chunks");
    dest->chunks = region::parse_chunks(raw_chunks);
    provided_keys_for_arg_parser->emplace("chunks");
  }
  if (rc.contains("min_x") && rc.at("min_x").is_integer()) {
    dest->min_x = rc["min_x"].as_integer();
    provided_keys_for_arg_parser->emplace("min-x");
  }
  if (rc.contains("max_x") && rc.at("max_x").is_integer()) {
    dest->max_x = rc["max_x"].as_integer();
    provided_keys_for_arg_parser->emplace("max-x");
  }
  if (rc.contains("min_z") && rc.at("min_z").is_integer()) {
    dest->min_z = rc["min_z"].as_integer();
    provided_keys_for_arg_parser->emplace("min-z");
  }
  if (rc.contains("max_z") && rc.at("max_z").is_integer()) {
    dest->max_z = rc["max_z"].as_integer();
    provided_keys_for_arg_parser->emplace("max-z");
  }
  if (rc.contains("num_threads") && rc.at("num_threads").is_integer()) {
    dest->num_threads = static_cast<i32>(rc["num_threads"].as_integer());
    provided_keys_for_arg_parser->emplace("num-threads");
  }
  if (rc.contains("allow_whole_rollback") &&
      rc.at("allow_whole_rollback").is_boolean()) {
    dest->allow_whole_rollback = rc["allow_whole_rollback"].as_boolean();
    provided_keys_for_arg_parser->emplace("allow-whole-rollback");
  }
  if (rc.contains("bulk_copy") && rc.at("bulk_copy").is_boolean()) {
    dest->bulk_copy = rc["bulk_copy"].as_boolean();
    provided_keys_for_arg_parser->emplace("bulk-copy");
  }
  if (rc.contains("dry_run") && rc.at("dry_run").is_boolean()) {
    dest->dry_run = rc["dry_run"].as_boolean();
    provided_keys_for_arg_parser->emplace("dry-run");
  }
  if (rc.contains("silent") && rc.at("silent").is_boolean()) {
    dest->silent = rc["silent"].as_boolean();
    provided_keys_for_arg_parser->emplace("silent");
  }
  if (rc.contains("verbose") && rc.at("verbose").is_boolean()) {
    dest->verbose = rc["verbose"].as_boolean();
    provided_keys_for_arg_parser->emplace("verbose");
  }

  return TomlParseStatus::Success;
}

}  // namespace app
