#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "audio_system.hpp"
#include "render_system.hpp"

#define GRID_CELL_SIZE (registry.screen_state.grid_cell_size)
#define WINDOW_WIDTH_PX (registry.screen_state.resolution_x)
#define WINDOW_HEIGHT_PX (registry.screen_state.resolution_y)

Entity createProjectile(vec2 pos, vec2 size, vec2 velocity, float angle, float angle_velocity, float damage, bool shot_by_player, TEXTURE_ASSET_ID texture_id, bool is_gun = false, bool can_bounce = false, int penetration_count = 0, int ricochets = 0);

Entity spawn_pickup(vec2 position, float angle, PICKUP_TYPE pickup_type, float value, GUN_TYPE gun_type = GUN_TYPE::GUN_COUNT);

void collect_pickup(Entity pickup_entity, AudioSystem* audio);

vec2 grid_to_world_coord(float x, float y);

ivec2 world_to_grid_coords(float x, float y);

void create_debris(vec2 door_loc, vec2 away_from_player, int rng);

