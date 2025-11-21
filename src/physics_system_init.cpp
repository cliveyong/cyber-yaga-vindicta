#include "physics_system_init.hpp"

ivec2 world_pos_to_hash_cell(SpatialHash& hash, vec2 pos) {
	float cell_size = hash.cell_size;
	int x = static_cast<int>(std::floor(pos.x / cell_size));
    int y = static_cast<int>(std::floor(pos.y / cell_size));

	x = std::max(0, std::min(x, hash.width - 1));
    y = std::max(0, std::min(y, hash.height - 1));
    
    return {x, y};
}

std::vector<ivec2> get_cells_for_entity(SpatialHash& hash, const Motion& motion) {
	std::vector<ivec2> cells;
	
	ivec2 top_left = world_pos_to_hash_cell(hash, { motion.position.x - motion.scale.x / 2., motion.position.y - motion.scale.y / 2. });
	ivec2 bottom_right = world_pos_to_hash_cell(hash, 
		{ motion.position.x + motion.scale.x / 2.,
		motion.position.y + motion.scale.y / 2. }
	);
	
	for (int x = top_left.x; x <= bottom_right.x; ++x) {
		for (int y = top_left.y; y <= bottom_right.y; ++y) {
			cells.push_back({x, y});
		}
	}
	
	return cells;
}

void add_statics_to_hash(SpatialHash& hash) {
	for (Entity static_entity : registry.staticCollidables.entities) {
		Motion& motion = registry.motions.get(static_entity);
		std::vector<ivec2> cells = get_cells_for_entity(hash, motion);
		for (auto& cell : cells) {
			hash.grid[cell.y][cell.x].push_back(static_entity);
		}
	}
}

std::vector<Entity> get_potential_collisions(SpatialHash& hash, Entity entity, Motion& motion) {
	std::vector<Entity> candidates;
	candidates.reserve(10);
	std::unordered_map<Entity, bool, EntityHash> visited;

	std::vector<ivec2> cells = get_cells_for_entity(hash, motion);

	for (const auto& cell : cells) {
		std::vector<Entity> entities = get_entities_in_cell(hash, cell);
		for (Entity& other_entity : entities) {
			if (other_entity == entity || visited[other_entity]) {
				continue;
			}
			
			candidates.push_back(other_entity);
			visited[other_entity] = true;
		}

	}
	return candidates;
}

std::vector<Entity> get_entities_in_cell(SpatialHash& hash, ivec2 pos) {
	return hash.grid[pos.y][pos.x];
}

void clear_and_set_spatial_hash() {
	while (!registry.spatialHashes.entities.empty()) {
		registry.remove_all_components_of(registry.spatialHashes.entities.back());
	}
	if (registry.maps.size() == 0) {
		return;
	}

	int current_level = registry.gameProgress.components[0].level;
	Map& map = registry.maps.components[current_level];
	SpatialHash& hash = registry.spatialHashes.emplace(Entity());
	hash.height = std::ceil((map.grid_height) * GRID_CELL_SIZE) / hash.cell_size;
	hash.width = std::ceil((map.grid_width) * GRID_CELL_SIZE) / hash.cell_size;
	hash.grid.resize(hash.height, std::vector<std::vector<Entity>>(hash.width));
	add_statics_to_hash(hash);
}
