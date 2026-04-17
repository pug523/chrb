#pragma once

#include <string>

#include "region/dimension.h"
#include "region/region_position.h"
#include "region/rollback_type.h"
#include "region/world_directory_structure.h"

namespace region {

WorldDirectoryStructure detect_world_structure(
    const std::string_view world_directory);
WorldDirectoryStructure world_directory_structure_with_config(
    const std::string_view world_dir,
    WorldDirectoryStructureConfig config);

void build_mca_dir_path(std::string* world_dir,
                        WorldDirectoryStructureConfig structure,
                        Dimension dimension,
                        RollbackType type);
void build_mca_file_path(std::string* mca_dir, RegionPosition region_pos);

bool parse_region_filename(const std::string& path, i32* out_rx, i32* out_rz);

}  // namespace region
