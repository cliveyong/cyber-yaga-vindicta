#pragma once
#include "common.hpp"

#include "tinyECS/entity.hpp"
#include "render_system.hpp"

class MapSystem {
public:
	Map* current_map;

    void init(RenderSystem* renderer);

	void load_map(int map_entity_index);

	void set_active_map(Map& map);

	void load_next_map();

	void render_map();

	void derender_map();

	void find_and_create_wall_sections();

	void find_and_create_shadow_casters();

	void create_wall_section(int start_x, int start_y, int end_x, int end_y);

	void spawn_map_pickups();

	void spawn_map_enemies();

	void make_door(Entity door_entity, Motion& motion, Map& map, bool vertical_door, int size);

	void make_narrow_vertical_door(Entity door_entity, Motion motion, Map& map);

	void make_narrow_horizontal_door(Entity door_entity, Motion motion, Map& map);

	void make_horizontal_door(Entity door_entity, Motion motion, Map& map);

	void make_vertical_door(Entity door_entity, Motion motion, Map& map);

private:

	RenderSystem* renderer;
};