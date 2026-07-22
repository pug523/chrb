// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/toml/toml_parser.h"

#include <string>
#include <string_view>
#include <toml.hpp>
#include <vector>

#include "app/driver.h"
#include "core/check.h"
#include "core/cli/console.h"
#include "core/file_util.h"
#include "region/chunk_position.h"

namespace app {

TomlParseStatus parse_toml_config(const std::string& config_path,
                                  region::RollbackConfig* dest) {
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

  if (rc.at("src_world").is_string()) {
    dest->src_world = rc["src_world"].as_string();
  }
  if (rc.at("dest_world").is_string()) {
    dest->dest_world = rc["dest_world"].as_string();
  }
  if (rc.at("dimension").is_string()) {
    dest->dim_str = rc["dimension"].as_string();
  }
  if (rc.at("rollback_type").is_string()) {
    dest->type_str = rc["rollback_type"].as_string();
  }
  if (rc.at("color").is_string()) {
    dest->color_str = rc["color"].as_string();
  }
  if (rc.at("src_world_structure").is_string()) {
    dest->src_world_structure_str = rc["src_world_structure"].as_string();
  }
  if (rc.at("dest_world_structure").is_string()) {
    dest->src_world_structure_str = rc["dest_world_structure"].as_string();
  }
  if (rc.at("chunks").is_array()) {
    const auto raw_chunks =
        toml::find<std::vector<std::vector<i32>>>(rc, "chunks");
    dest->chunks = region::parse_chunks(raw_chunks);
  }
  if (rc.at("min_x").is_integer()) {
    dest->min_x = rc["min_x"].as_integer();
  }
  if (rc.at("max_x").is_integer()) {
    dest->max_x = rc["max_x"].as_integer();
  }
  if (rc.at("min_z").is_integer()) {
    dest->min_z = rc["min_z"].as_integer();
  }
  if (rc.at("max_z").is_integer()) {
    dest->max_z = rc["max_z"].as_integer();
  }
  if (rc.at("num_threads").is_integer()) {
    dest->num_threads = static_cast<i32>(rc["num_threads"].as_integer());
  }
  if (rc.at("verbose").is_boolean()) {
    dest->verbose = rc["verbose"].as_boolean();
  }

  return TomlParseStatus::Success;
}

}  // namespace app
