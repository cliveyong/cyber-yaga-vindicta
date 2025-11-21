#include "common.hpp"
#include "world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

ivec2 world_pos_to_hash_cell(SpatialHash& hash, vec2 pos);

std::vector<ivec2> get_cells_for_entity(SpatialHash& hash, const Motion& motion);

std::vector<Entity> get_potential_collisions(SpatialHash& hash, Entity entity, Motion& motion);

void add_statics_to_hash(SpatialHash& hash);

std::vector<Entity> get_entities_in_cell(SpatialHash& hash, ivec2 pos);

void clear_and_set_spatial_hash();
