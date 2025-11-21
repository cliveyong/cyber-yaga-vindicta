#include "map_system.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/registry.hpp"
#include "physics_system_init.hpp"
#include "ai_system_init.hpp"
#include "input_system.hpp"
#include <cmath>
#include <iostream>
#include "map_init.hpp"
#include "animation_init.hpp"
#include "player_system.hpp"

void MapSystem::init(RenderSystem* renderer) {
	this->renderer = renderer;
	create_map_0();
	create_map_1();
	create_map_2();
	create_map_3();
	load_map(0);
}

void MapSystem::load_map(int map_entity_index) {
	Map& map = registry.maps.components[map_entity_index];
	update_tile_grid(map);
	set_active_map(map);
	render_map();
	spawn_map_pickups();
	spawn_map_enemies();
}

void MapSystem::set_active_map(Map& map) {
	current_map = &map;
}

void MapSystem::load_next_map() {
	if (registry.gameProgress.components[0].level+1 == registry.maps.entities.size()) {
		create_animation(
			{ registry.screen_state.resolution_x / 2, registry.screen_state.resolution_y / 2 },
			{ 0.f, 0.f },
			{ registry.screen_state.resolution_x, registry.screen_state.resolution_y },
			0.f,
			std::vector<TEXTURE_ASSET_ID>{
				TEXTURE_ASSET_ID::GAME_END_TEXT
			},
			true,
			true,
			3000.0f,
			Z_INDEX::CUTSCENE,
			true,
			true
		);
		std::cout << "Max level reached" << std::endl;
		return;
	}
	derender_map();
	GameProgress& game_progress = registry.gameProgress.components[0];
	game_progress.level++;
	load_map(game_progress.level);

	if (!debugging.disable_music) {
		registry.audio_system->play_music(current_map->music, 14);
	}

	create_animation(
		{registry.screen_state.resolution_x/2, registry.screen_state.resolution_y/2},
		{ 0.f, 0.f },
		{registry.screen_state.resolution_x, registry.screen_state.resolution_y},
		0.f,
		std::vector<TEXTURE_ASSET_ID>{
			current_map->level_title,
		},
		true,
		false,
		3000.0f,
		Z_INDEX::CUTSCENE,
		true,
		true
	);

	Entity player_entity = registry.players.entities[0];
	Player& player_component = registry.players.get(player_entity);
	player_component.health = STARTING_PLAYER_HEALTH;
	Motion& player_motion = registry.motions.get(player_entity);
	player_motion.position = grid_to_world_coord(current_map->start_location.x, current_map->start_location.y);
	registry.guns.remove(player_entity);
	create_gun(player_entity, current_map->gun_type);
	registry.ui_system->update_gun_ui();
	update_player_sprite();
}

void MapSystem::make_narrow_vertical_door(Entity door_entity, Motion motion, Map& map) {
	Entity e1 = Entity();
	Motion& m1 = registry.motions.insert(e1, motion);
	Entity e2 = Entity();
	Motion& m2 = registry.motions.insert(e2, motion);
	Entity e3 = Entity();
	Motion& m3 = registry.motions.insert(e3, motion);

	m1.position = { m1.position.x - 4, m1.position.y + 97 };
	m2.position = { m2.position.x - 4, m2.position.y - 152 };
	m3.position = { m3.position.x - 4, m3.position.y - 29 };

	m3.scale = { m3.scale.x * .8, m3.scale.y * .5 };
	m1.scale = { m1.scale.x * .8, m1.scale.y * .5 };
	m2.scale = { m2.scale.x * .8, m2.scale.y * .5 };

	Entity e4 = Entity();
	Motion& m4 = registry.motions.insert(e4, motion);
	Entity e5 = Entity();
	Motion& m5 = registry.motions.insert(e5, motion);
	Entity e6 = Entity();
	Motion& m6 = registry.motions.insert(e6, motion);
	m4.position = { m4.position.x + 64, m4.position.y - 152 };
	m5.position = { m5.position.x + 64, m5.position.y + 97 };
	m6.position = { m6.position.x + 64, m6.position.y - 29 };

	m4.angle += 180;
	m5.angle += 180;
	m6.angle += 180;

	m6.scale = { m6.scale.x * .8, m6.scale.y * .5 };
	m4.scale = { m4.scale.x * .8, m4.scale.y * .5 };
	m5.scale = { m5.scale.x * .8, m5.scale.y * .5 };

	Entity e7 = Entity();
	Motion& m7 = registry.motions.insert(e7, motion);
	m7.position = { m7.position.x + 65, m7.position.y - 28};
	m7.angle += 180;

	registry.renderRequests.insert(
		e1,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	registry.renderRequests.insert(
		e2,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e3,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e4,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e5,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e6,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);

	registry.renderRequests.insert(
		e7,
		{
			TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_NORMAL
		}
	);

	map.prop_doors_pair.push_back({ door_entity, {e1, e2, e3, e4, e5, e6, e7}});
	for (auto& pair : map.prop_doors_pair) {
		for (auto& entity : pair.second) {
			current_map->rendered_entities.push_back(entity);
		}
	}
}

void MapSystem::make_narrow_horizontal_door(Entity door_entity, Motion motion, Map& map) {

	Entity e1 = Entity();
	Motion& m1 = registry.motions.insert(e1, motion);
	Entity e2 = Entity();
	Motion& m2 = registry.motions.insert(e2, motion);
	Entity e3 = Entity();
	Motion& m3 = registry.motions.insert(e3, motion);

	m1.position = { m1.position.x + 165, m1.position.y + 4 };
	m2.position = { m2.position.x - 107, m2.position.y + 2 };
	m3.position = { m3.position.x + 20, m3.position.y + 3 };

	m3.scale = { m3.scale.x * .5, m3.scale.y * .8 };
	m1.scale = { m1.scale.x * .5, m1.scale.y * .8 };
	m2.scale = { m2.scale.x * .5, m2.scale.y * .8 };

	Entity e4 = Entity();
	Motion& m4 = registry.motions.insert(e4, motion);
	Entity e5 = Entity();
	Motion& m5 = registry.motions.insert(e5, motion);
	Entity e6 = Entity();
	Motion& m6 = registry.motions.insert(e6, motion);
	m4.position = { m4.position.x - 105, m4.position.y - 62 };
	m5.position = { m5.position.x + 167, m5.position.y - 60 };
	m6.position = { m6.position.x + 30 , m6.position.y - 61 };

	m4.angle += 180;
	m5.angle += 180;
	m6.angle += 180;

	m6.scale = { m6.scale.x * .5, m6.scale.y * .8 };
	m4.scale = { m4.scale.x * .5, m4.scale.y * .8 };
	m5.scale = { m5.scale.x * .5, m5.scale.y * .8 };

	Entity e7 = Entity();
	Motion& m7 = registry.motions.insert(e7, motion);
	m7.position = { m7.position.x + 29, m7.position.y - 65 };
	m7.angle += 180;

	registry.renderRequests.insert(
		e1,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	registry.renderRequests.insert(
		e2,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e3,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e4,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e5,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e6,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);
	registry.renderRequests.insert(
		e7,
		{
			TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_NORMAL
		}
	);

	map.prop_doors_pair.push_back({ door_entity, {e1, e2, e3, e4, e5, e6, e7} });

	for (auto& pair : map.prop_doors_pair) {
		for (auto& entity : pair.second) {
			current_map->rendered_entities.push_back(entity);
		}
	}
}

void MapSystem::make_vertical_door(Entity door_entity, Motion motion, Map& map) {
	Entity e1 = Entity();
	Motion& m1 = registry.motions.insert(e1, motion);
	Entity e2 = Entity();
	Motion& m2 = registry.motions.insert(e2, motion);
	Entity e3 = Entity();
	Motion& m3 = registry.motions.insert(e3, motion);

	m1.position = { m1.position.x - 4, m1.position.y + 208 };
	m2.position = { m2.position.x - 4, m2.position.y - 208 };
	m3.position = { m3.position.x - 4, m3.position.y + 4 };

	m3.scale = { m3.scale.x * .8, m3.scale.y * .5 };
	m1.scale = { m1.scale.x * .8, m1.scale.y * .5 };
	m2.scale = { m2.scale.x * .8, m2.scale.y * .5 };

	Entity e4 = Entity();
	Motion& m4 = registry.motions.insert(e4, motion);
	Entity e5 = Entity();
	Motion& m5 = registry.motions.insert(e5, motion);
	Entity e6 = Entity();
	Motion& m6 = registry.motions.insert(e6, motion);
	m4.position = { m4.position.x + 64, m4.position.y - 208 };
	m5.position = { m5.position.x + 64, m5.position.y + 208 };
	m6.position = { m6.position.x + 64, m6.position.y - 4 };

	m4.angle += 180;
	m5.angle += 180;
	m6.angle += 180;

	m6.scale = { m6.scale.x * .8, m6.scale.y * .5 };
	m4.scale = { m4.scale.x * .8, m4.scale.y * .5 };
	m5.scale = { m5.scale.x * .8, m5.scale.y * .5 };

	Entity e7 = Entity();
	Motion& m7 = registry.motions.insert(e7, motion);
	m7.position = { m7.position.x + 65, m7.position.y  };
	m7.angle += 180;

	registry.renderRequests.insert(
		e1,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	registry.renderRequests.insert(
		e2,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e3,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e4,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e5,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	
	registry.renderRequests.insert(
		e6,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);

	registry.renderRequests.insert(
		e7,
		{
			TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_NORMAL
		}
	);

	map.prop_doors_pair.push_back({ door_entity, {e1, e2, e3, e4, e5, e6, e7} });

	for (auto& pair : map.prop_doors_pair) {
		for (auto& entity : pair.second) {
			current_map->rendered_entities.push_back(entity);
		}
	}
}

void MapSystem::make_horizontal_door(Entity door_entity, Motion motion, Map& map) {
	
	Entity e1 = Entity();
	Motion& m1 = registry.motions.insert(e1, motion);
	Entity e2 = Entity();
	Motion& m2 = registry.motions.insert(e2, motion);
	Entity e3 = Entity();
	Motion& m3 = registry.motions.insert(e3, motion);

	m1.position = { m1.position.x + 207, m1.position.y + 5 };
	m2.position = { m2.position.x - 210, m2.position.y + 3 };
	m3.position = { m3.position.x, m3.position.y + 4 };

	m3.scale = { m3.scale.x * .5, m3.scale.y * .8 };
	m1.scale = { m1.scale.x * .5, m1.scale.y * .8 };
	m2.scale = { m2.scale.x * .5, m2.scale.y * .8 };

	Entity e4 = Entity();
	Motion& m4 = registry.motions.insert(e4, motion);
	Entity e5 = Entity();
	Motion& m5 = registry.motions.insert(e5, motion);
	Entity e6 = Entity();
	Motion& m6 = registry.motions.insert(e6, motion);
	m4.position = { m4.position.x - 207, m4.position.y - 65 };
	m5.position = { m5.position.x + 210, m5.position.y - 63 };
	m6.position = { m6.position.x , m6.position.y - 64 };

	m4.angle += 180;
	m5.angle += 180;
	m6.angle += 180;

	m6.scale = { m6.scale.x * .5, m6.scale.y * .8 };
	m4.scale = { m4.scale.x * .5, m4.scale.y * .8 };
	m5.scale = { m5.scale.x * .5, m5.scale.y * .8 };

	Entity e7 = Entity();
	Motion& m7 = registry.motions.insert(e7, motion);
	m7.position = { m7.position.x, m7.position.y - 65 };
	m7.angle += 180;

	registry.renderRequests.insert(
		e1,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	registry.renderRequests.insert(
		e2,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	registry.renderRequests.insert(
		e3,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);

	registry.renderRequests.insert(
		e4,
		{
			TEXTURE_ASSET_ID::DOOR_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_RIGHT_NORMAL
		}
	);
	registry.renderRequests.insert(
		e5,
		{
			TEXTURE_ASSET_ID::DOOR_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_LEFT_NORMAL
		}
	);
	registry.renderRequests.insert(
		e6,
		{
			TEXTURE_ASSET_ID::DOOR_TOP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::WALL,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_TOP_NORMAL
		}
	);

	registry.renderRequests.insert(
		e7,
		{
			TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_NORMAL
		}
	);

	map.prop_doors_pair.push_back({ door_entity, {e1, e2, e3, e4, e5, e6, e7} });
	for (auto& pair : map.prop_doors_pair) {
		for (auto& entity : pair.second) {
			current_map->rendered_entities.push_back(entity);
		}
	}
}

void MapSystem::make_door(Entity door_entity,Motion& motion, Map& map, bool vertical_door, int size) {

	if (size == 2) {
		if (vertical_door) {
			motion.angle += 90;
			make_vertical_door(door_entity, motion, map);
			motion.position = { motion.position.x - 2, motion.position.y };
		}
		else {
			make_horizontal_door(door_entity, motion, map);
			motion.position = { motion.position.x, motion.position.y + 7 };
		}
	}
	else {
		if (vertical_door) {
			motion.angle += 90;
			make_narrow_vertical_door(door_entity, motion, map);
			motion.position = { motion.position.x - 4, motion.position.y - 27};
		}
		else {
			make_narrow_horizontal_door(door_entity, motion, map);
			motion.position = { motion.position.x + 28, motion.position.y + 8 };
		}
	}
	registry.renderRequests.insert(
		door_entity,
		{
			TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::PROP,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			TEXTURE_ASSET_ID::DOOR_NORMAL
		}
	);
}

void MapSystem::render_map() {
	find_and_create_shadow_casters();
	renderer->set_map_texture(*current_map);
	
	Entity base_map_ent = Entity();
	Motion& motion = registry.motions.emplace(base_map_ent);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = { GRID_CELL_SIZE * current_map->grid_width * 0.5f, GRID_CELL_SIZE * current_map->grid_height * 0.5f};
	motion.scale = vec2({ GRID_CELL_SIZE * current_map->grid_width, GRID_CELL_SIZE * current_map->grid_height });

	registry.renderRequests.insert(
		base_map_ent,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::FLOOR,
			false,
			TEXTURE_ORIGIN_ID::MAP
		}
	);
	current_map->rendered_entities.push_back(base_map_ent);

	for (Light& light : current_map->lights) {
		Entity light_ent = Entity();
		Light& new_light = registry.lights.emplace(light_ent);
		new_light.color = light.color;
		new_light.position = light.position;
		new_light.radius = light.radius;
		new_light.intensity = light.intensity;
		new_light.height = light.height;

		current_map->rendered_entities.push_back(light_ent);
	}

	for (const auto& [pos, prop] : current_map->props) {
		Entity entity = Entity();
		Motion& motion = registry.motions.emplace(entity);
		motion.angle = prop.angle;
		motion.velocity = { 0.0f, 0.0f };
		motion.position = grid_to_world_coord(pos.x, pos.y);
		motion.scale = vec2({ GRID_CELL_SIZE * prop.size.x, GRID_CELL_SIZE * prop.size.y});

		// see if prop is door
		int current_level = registry.gameProgress.components[0].level;
		Map& map = registry.maps.components[current_level];
		bool found = false;
		bool is_vertical = false;
		int size = 1;
		for (const auto& door_loc : map.prop_doors_list) {
			if (door_loc.x == pos.x && door_loc.y == pos.y) {
				found = true;
				Prop door_prop = map.props[door_loc];
				is_vertical = door_prop.vertical;
				size = door_prop.prop_size;
				break;
			}
		}

		if (found) {
			map.prop_doors[pos] = entity;
			make_door(entity, motion, map, is_vertical, size);
		}
		else {
			registry.renderRequests.insert(
				entity,
				{
					prop.texture,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE,
					Z_INDEX::PROP,
					false,
					TEXTURE_ORIGIN_ID::FILE,
					prop.normal
				}
			);
		}

		if (prop.collidable) {
			registry.staticCollidables.emplace(entity);
			AABB& aabb = registry.AABBs.emplace(entity);
			aabb.collision_box = prop.collision_size * (float)GRID_CELL_SIZE;
			aabb.offset = prop.collision_offset * (float)GRID_CELL_SIZE;
			if (found) {
				current_map->tile_id_grid[pos.y][pos.x] = TILE_ID::CLOSED_DOOR;
			}
			else {
				current_map->tile_id_grid[pos.y][pos.x] = TILE_ID::WALL;
			}
		}

		current_map->rendered_entities.push_back(entity);
		
	}
	
	find_and_create_wall_sections();
	current_map->information_props = current_map->props;
	clear_and_set_spatial_hash();
}

void MapSystem::derender_map() {
	for (auto& entity : current_map->rendered_entities) {
		registry.remove_all_components_of(entity);
	}

	while (!registry.staticCollidables.entities.empty()) {
		registry.remove_all_components_of(registry.staticCollidables.entities.back());
	}

	while (!registry.shadowCasters.entities.empty()) {
		registry.remove_all_components_of(registry.shadowCasters.entities.back());
	}

	while (!registry.pickups.entities.empty()) {
		registry.remove_all_components_of(registry.pickups.entities.back());
	}

	while (!registry.enemies.entities.empty()) {
		registry.remove_all_components_of(registry.enemies.entities.back());
	}

	while (!registry.deadEnemies.entities.empty()) {
		registry.remove_all_components_of(registry.deadEnemies.entities.back());
	}

	while (!registry.instructionMessages.entities.empty()) {
		registry.remove_all_components_of(registry.instructionMessages.entities.back());
	}

	current_map->rendered_entities.clear();
}

void MapSystem::find_and_create_shadow_casters() {
    int rows = current_map->grid_height;
    int cols = current_map->grid_width;
    std::vector<std::vector<TILE_ID>>& tile_id_grid = current_map->tile_id_grid;
    std::vector<std::vector<WALL_DIRECTION>>& wall_directions = current_map->wall_directions;
    
	for (int y = 0; y < rows; y++) {
		bool in_segment = false;
		int segment_start_x = 0;
		bool is_top_edge = false;
		
		for (int x = 0; x <= cols; x++) {
			bool is_valid_edge = false;
			bool current_is_top_edge = false;
			
			if (x < cols && tile_id_grid[y][x] == TILE_ID::WALL) {
				WALL_DIRECTION dir = wall_directions[y][x];
				
				if (dir == WALL_DIRECTION::TOP || 
					dir == WALL_DIRECTION::TOP_RIGHT || 
					dir == WALL_DIRECTION::TOP_LEFT) {
					is_valid_edge = true;
					current_is_top_edge = true;
				}
				else if (dir == WALL_DIRECTION::BOTTOM || 
						 dir == WALL_DIRECTION::BOTTOM_RIGHT || 
						 dir == WALL_DIRECTION::BOTTOM_LEFT) {
					is_valid_edge = true;
					current_is_top_edge = false;
				}
			}
			
			if (in_segment && (!is_valid_edge || current_is_top_edge != is_top_edge)) {
				ShadowCaster& shadow_caster = registry.shadowCasters.emplace(Entity());
				
				if (is_top_edge) {
					shadow_caster.start = grid_to_world_coord(segment_start_x, y) + vec2(-GRID_CELL_SIZE / 2.0, -GRID_CELL_SIZE / 2.0);
					shadow_caster.end = grid_to_world_coord(x - 1, y) + vec2(GRID_CELL_SIZE / 2.0, -GRID_CELL_SIZE / 2.0);
				} else {
					shadow_caster.start = grid_to_world_coord(segment_start_x, y) + vec2(-GRID_CELL_SIZE / 2.0, GRID_CELL_SIZE / 2.0);
					shadow_caster.end = grid_to_world_coord(x - 1, y) + vec2(GRID_CELL_SIZE / 2.0, GRID_CELL_SIZE / 2.0);
				}
				
				in_segment = false;
			}
			
			if (!in_segment && is_valid_edge) {
				segment_start_x = x;
				is_top_edge = current_is_top_edge;
				in_segment = true;
			}
		}
	}
    
    for (int x = 0; x < cols; x++) {
        bool in_segment = false;
        int segment_start_y = 0;
        
        for (int y = 0; y <= rows; y++) {
            bool is_valid_edge = (y < rows) && 
                               (tile_id_grid[y][x] == TILE_ID::WALL) && 
                               (wall_directions[y][x] == WALL_DIRECTION::LEFT ||
                                wall_directions[y][x] == WALL_DIRECTION::TOP_LEFT ||
                                wall_directions[y][x] == WALL_DIRECTION::BOTTOM_LEFT);
            
            if (in_segment && !is_valid_edge) {
                ShadowCaster& shadow_caster = registry.shadowCasters.emplace(Entity());
                shadow_caster.start = grid_to_world_coord(x, segment_start_y) + vec2(-GRID_CELL_SIZE / 2.0, -GRID_CELL_SIZE / 2.0);
                shadow_caster.end = grid_to_world_coord(x, y - 1) + vec2(-GRID_CELL_SIZE / 2.0, GRID_CELL_SIZE / 2.0);
                
                in_segment = false;
            }
            
            if (!in_segment && is_valid_edge) {
                segment_start_y = y;
                in_segment = true;
            }
        }
    }
    
    for (int x = 0; x < cols; x++) {
        bool in_segment = false;
        int segment_start_y = 0;
        
        for (int y = 0; y <= rows; y++) {
            bool is_valid_edge = (y < rows) && 
                               (tile_id_grid[y][x] == TILE_ID::WALL) && 
                               (wall_directions[y][x] == WALL_DIRECTION::RIGHT ||
                                wall_directions[y][x] == WALL_DIRECTION::TOP_RIGHT ||
                                wall_directions[y][x] == WALL_DIRECTION::BOTTOM_RIGHT);
            
            if (in_segment && !is_valid_edge) {
                ShadowCaster& shadow_caster = registry.shadowCasters.emplace(Entity());
                shadow_caster.start = grid_to_world_coord(x, segment_start_y) + vec2(GRID_CELL_SIZE / 2.0, -GRID_CELL_SIZE / 2.0);
                shadow_caster.end = grid_to_world_coord(x, y - 1) + vec2(GRID_CELL_SIZE / 2.0, GRID_CELL_SIZE / 2.0);
                
                in_segment = false;
            }
            
            if (!in_segment && is_valid_edge) {
                segment_start_y = y;
                in_segment = true;
            }
        }
    }
}

void MapSystem::find_and_create_wall_sections() {
	std::vector<std::vector<bool>> visited(current_map->grid_height, std::vector<bool>(current_map->grid_width, false));
	int height = current_map->grid_height;
	int width = current_map->grid_width;
	std::vector<std::vector<TILE_ID>>& grid = current_map->tile_id_grid;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			if (grid[row][col] != TILE_ID::WALL || visited[row][col]) {
				continue;
			}

			int end_col = col;
			while (end_col + 1 < width && !visited[row][end_col + 1] && grid[row][end_col + 1] == TILE_ID::WALL) {
				end_col++;
			}
			if (end_col > col) {
				for (int i = col; i <= end_col; i++) {
					visited[row][i] = true;
				}
				create_wall_section(col, row, end_col, row);
				continue;
			}


			int end_row = row;
			while (end_row + 1 < height && !visited[end_row + 1][col] && grid[end_row + 1][col] == TILE_ID::WALL) {
				end_row++;
			}
			if (end_row > row) {
				for (int i = row; i <= end_row; i++) {
					visited[i][col] = true;
				}
				create_wall_section(col, row, col, end_row);
				continue;
			}

			visited[row][col] = true;
			create_wall_section(col, row, col, row);
		}
	}
}

void MapSystem::create_wall_section(int start_x, int start_y, int end_x, int end_y) {
	Entity wall_ent = Entity();
	StaticCollidable& wall = registry.staticCollidables.emplace(wall_ent);
	Motion& motion = registry.motions.emplace(wall_ent);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = (grid_to_world_coord(start_x, start_y) + grid_to_world_coord(end_x, end_y))/2.f;
	motion.scale = vec2({ (end_x - start_x + 1) * GRID_CELL_SIZE, (end_y - start_y + 1) * GRID_CELL_SIZE});

	registry.collidables.emplace(wall_ent);
	AABB& aabb = registry.AABBs.emplace(wall_ent);
	aabb.collision_box = motion.scale;
	aabb.offset = { 0.f, 0.f };

	current_map->rendered_entities.push_back(wall_ent);
}

void MapSystem::spawn_map_pickups() {
	std::vector<Pickup> pickups = current_map->pickups;
	for (Pickup pickup: pickups) {
		spawn_pickup(grid_to_world_coord(pickup.grid_pos.x, pickup.grid_pos.y), 0, pickup.type, pickup.value, pickup.gun_type);
	}
}

void MapSystem::spawn_map_enemies() {
	std::vector<EnemyBlueprint> enemy_blueprints = current_map->enemy_blueprints;
	for (EnemyBlueprint enemy_blueprint: enemy_blueprints) {
		create_enemy(enemy_blueprint.grid_position, enemy_blueprint.gun_type, enemy_blueprint.health, enemy_blueprint.speed_factor, enemy_blueprint.detection_range_factor, enemy_blueprint.attack_range_factor);
	}
}