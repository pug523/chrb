// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/path_util.h"

#include <cstddef>
#include <format>
#include <string>
#include <string_view>

#include "core/check.h"
#include "core/core.h"
#include "core/file_util.h"
#include "region/dimension.h"
#include "region/region_position.h"
#include "region/rollback_type.h"
#include "region/world_directory_structure.h"

namespace region {

WorldDirectoryStructure detect_world_structure(
    const std::string_view world_directory) {
  DCHECK(core::is_path_delimiter(world_directory.back()));

  std::string ow_new(world_directory);
  std::string ne_new(world_directory);
  std::string end_new(world_directory);
  ow_new.append(dimension_path_with_slash_new(Dimension::OverWorld))
      .append(type_path(RollbackType::Region));
  ne_new.append(dimension_path_with_slash_new(Dimension::Nether))
      .append(type_path(RollbackType::Region));
  end_new.append(dimension_path_with_slash_new(Dimension::End))
      .append(type_path(RollbackType::Region));
  if (core::is_dir(ow_new) || core::is_dir(ne_new) || core::is_dir(end_new)) {
    return WorldDirectoryStructure::New;
  }

  std::string ow_old(world_directory);
  std::string ne_old(world_directory);
  std::string end_old(world_directory);
  ow_old.append(dimension_path_with_slash_old(Dimension::OverWorld))
      .append(type_path(RollbackType::Region));
  ne_old.append(dimension_path_with_slash_old(Dimension::Nether))
      .append(type_path(RollbackType::Region));
  end_old.append(dimension_path_with_slash_old(Dimension::End))
      .append(type_path(RollbackType::Region));
  if (core::is_dir(ow_old) || core::is_dir(ne_old) || core::is_dir(end_old)) {
    return WorldDirectoryStructure::Old;
  }

  std::string ow_paper(world_directory);
  std::string ne_paper(world_directory);
  std::string end_paper(world_directory);
  ow_paper.append(dimension_path_with_slash_paper(Dimension::OverWorld))
      .append(type_path(RollbackType::Region));
  ne_paper.append(dimension_path_with_slash_paper(Dimension::Nether))
      .append(type_path(RollbackType::Region));
  end_paper.append(dimension_path_with_slash_paper(Dimension::End))
      .append(type_path(RollbackType::Region));
  if (core::is_dir(ow_paper) || core::is_dir(ne_paper) ||
      core::is_dir(end_paper)) {
    return WorldDirectoryStructure::Paper;
  }

  return WorldDirectoryStructure::Unknown;
}

WorldDirectoryStructure world_directory_structure_with_config(
    const std::string_view world_dir,
    WorldDirectoryStructureConfig config) {
  DCHECK(core::is_path_delimiter(world_dir.back()));
  switch (config) {
    using WC = WorldDirectoryStructureConfig;
    case WC::Auto: return detect_world_structure(world_dir);
    case WC::New: return WorldDirectoryStructure::New;
    case WC::Old: return WorldDirectoryStructure::Old;
    case WC::Paper: return WorldDirectoryStructure::Paper;
    default: return WorldDirectoryStructure::Unknown;
  }
}

void build_mca_dir_path(std::string* world_dir,
                        WorldDirectoryStructureConfig structure,
                        Dimension dimension,
                        RollbackType type) {
  DCHECK(core::is_path_delimiter(world_dir->back()));
  DCHECK(structure != WorldDirectoryStructureConfig::Unknown);
  DCHECK(dimension != Dimension::Unknown);
  DCHECK(type != RollbackType::Unknown);

  WorldDirectoryStructure s =
      world_directory_structure_with_config(*world_dir, structure);

  switch (s) {
    case WorldDirectoryStructure::New: {
      world_dir->append(dimension_path_with_slash_new(dimension))
          .append(type_path(type))
          .push_back(PATH_DELIMITER);
      return;
    }
    case WorldDirectoryStructure::Old: {
      world_dir->append(dimension_path_with_slash_old(dimension))
          .append(type_path(type))
          .push_back(PATH_DELIMITER);
      return;
    }
    case WorldDirectoryStructure::Paper: {
      world_dir->append(dimension_path_with_slash_paper(dimension))
          .append(type_path(type))
          .push_back(PATH_DELIMITER);
      return;
    }
    default: {
      return;
    }
  }
}

void build_mca_file_path(std::string* mca_dir, RegionPosition region_pos) {
  DCHECK(core::is_path_delimiter(mca_dir->back()));
  mca_dir->append(std::format("r.{}.{}.mca", region_pos.x, region_pos.z));
}

bool parse_region_filename(const std::string& path, i32* out_rx, i32* out_rz) {
  // find the last '/' or '\' to isolate the filename
  const size_t slash = path.find_last_of("/\\");
  const size_t name_start = (slash == std::string::npos) ? 0 : slash + 1;
  const std::string_view name(path.data() + name_start,
                              path.size() - name_start);

  // expect "r.<x>.<z>.mca"
  if (name.size() < 9 || name[0] != 'r' || name[1] != '.') {
    return false;
  }

  const size_t dot1 = name.find('.', 2);
  if (dot1 == std::string_view::npos) {
    return false;
  }
  const size_t dot2 = name.find('.', dot1 + 1);
  if (dot2 == std::string_view::npos) {
    return false;
  }
  // must end with ".mca"
  if (name.substr(dot2 + 1) != "mca") {
    return false;
  }

  const std::string_view x_str = name.substr(2, dot1 - 2);
  const std::string_view z_str = name.substr(dot1 + 1, dot2 - dot1 - 1);

  // simple integer parse (avoids std::stoi and locale overhead)
  auto parse_int = [](std::string_view s, i32* out) -> bool {
    if (s.empty()) {
      return false;
    }
    bool neg = false;
    size_t i = 0;
    if (s[0] == '-') {
      neg = true;
      ++i;
    }
    if (i == s.size()) {
      return false;
    }
    i32 v = 0;
    for (; i < s.size(); ++i) {
      if (s[i] < '0' || s[i] > '9') {
        return false;
      }
      v = v * 10 + (s[i] - '0');
    }
    *out = neg ? -v : v;
    return true;
  };

  return parse_int(x_str, out_rx) && parse_int(z_str, out_rz);
}

}  // namespace region
