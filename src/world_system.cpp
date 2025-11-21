// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "audio_system.hpp"
#include "physics_system.hpp"
#include "player_system.hpp"
#include "input_system.hpp"
#include "map_system.hpp"
#include "ai_system_init.hpp"
#include "animation_init.hpp"
#include "physics_system_init.hpp"
#include "ui_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <glm/trigonometric.hpp>
#include <map_init.hpp>
#include <props.hpp>



// create the world
WorldSystem::WorldSystem()
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
	registry.screen_state.resolution_x = mode->width;
	registry.screen_state.resolution_y = mode->height;
	registry.screen_state.grid_cell_size = mode->height / GRID_HEIGHT;
	if (debugging.disable_fullscreen) {
		registry.screen_state.resolution_y -= 100; // To give enough room for Apple menubar and dock
	}
	GLFWmonitor* monitor_ptr = debugging.disable_fullscreen ? nullptr : primary_monitor;
	window = glfwCreateWindow(registry.screen_state.resolution_x, registry.screen_state.resolution_y, "Cyber-Yaga Vindicta", monitor_ptr, nullptr);

	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, AudioSystem* audio) {
 

	this->renderer = renderer_arg;
	this->audio = audio;

	// Need to check here what the game state is but that currently crashes the game

	// start playing background music indefinitely
	// std::cout << "Starting music..." << std::endl;
	//if (!debugging.disable_music) {
	//	audio->play_music(MUSIC_ASSET_ID::MUSIC1, 7);
	//}

	Entity game_progress_entity = Entity();
	GameProgress game_progress = registry.gameProgress.emplace(game_progress_entity);
	game_progress.level = 0;

	// Set all states to default
	display_instruction_images();
    restart_game();
}

void WorldSystem::progress_timers(float elapsed_ms) {
	for (int i = 0; i < registry.removeTimers.size(); i++) {
		RemoveTimer& timer = registry.removeTimers.components[i];
		timer.remaining_time -= elapsed_ms;
		if (timer.remaining_time < 0.) {
			registry.remove_all_components_of(registry.removeTimers.entities[i]);
		}
	}

	ScreenState& screen = registry.screen_state;
	if (screen.glitch_remaining_ms > 0.f) {
		screen.glitch_remaining_ms -= elapsed_ms;
	}
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	progress_timers(elapsed_ms_since_last_update);

	Input& input = registry.inputs.components[0];
	if (input.keys[GLFW_KEY_P]) {
		if (game_state == GameState::CINEMATIC) {
			//TODO: make it so it skips the cinematic
		}
		else {
			restart_game();
			input.keys[GLFW_KEY_P] = false;
		}
		
	}
	

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	while (!registry.enemies.entities.empty()) {
		registry.remove_all_components_of(registry.enemies.entities.back());
	}
	while (!registry.deadEnemies.entities.empty()) {
		registry.remove_all_components_of(registry.deadEnemies.entities.back());
	}
	while (!registry.projectiles.entities.empty()) {
		registry.remove_all_components_of(registry.projectiles.entities.back());
	}
	while (!registry.pickups.entities.empty()) {
		registry.remove_all_components_of(registry.pickups.entities.back());
	}

	

	if (!registry.players.entities.empty()) {
		Entity& player = registry.players.entities[0];
		Player& player_component = registry.players.components[0];
		player_component.health = STARTING_PLAYER_HEALTH;
		
		if (registry.motions.has(player)) { //TODO: in future, add start_position to the Map
			auto& playerMotion = registry.motions.get(player);
			vec2 grid_start_location = registry.map_system->current_map->start_location;
			playerMotion.position = grid_to_world_coord(grid_start_location.x, grid_start_location.y);
		}

		if (registry.guns.has(player)) {
			registry.guns.remove(player);
		}
		create_gun(player, registry.map_system->current_map->gun_type);
		registry.ui_system->update_gun_ui();

		// Reset enemies and pickups
		registry.map_system->spawn_map_pickups();
		registry.map_system->spawn_map_enemies();
	}
	else {
		renderer->initializeCamera();
	}

	if (registry.maps.size() != 0) {
		int current_level = registry.gameProgress.components[0].level;
		Map& map = registry.maps.components[current_level];

		for (const auto& [pos, prop] : map.information_props) {
			auto it = map.props.find(pos);
			if (it == map.props.end()) {
				
				Entity entity = Entity();
				Motion& motion = registry.motions.emplace(entity);
				motion.angle = prop.angle;
				motion.velocity = { 0.0f, 0.0f };
				motion.position = grid_to_world_coord(pos.x, pos.y);
				motion.scale = vec2({ GRID_CELL_SIZE * prop.size.x, GRID_CELL_SIZE * prop.size.y });
				map.props[pos] = prop;
				map.prop_doors[pos] = entity;
				map.prop_doors_list.push_back(pos);

				registry.map_system->make_door(entity, motion, map, prop.vertical, prop.prop_size);
				
				registry.staticCollidables.emplace(entity);
				AABB& aabb = registry.AABBs.emplace(entity);
				aabb.collision_box = prop.collision_size * (float)GRID_CELL_SIZE;
				aabb.offset = prop.collision_offset * (float)GRID_CELL_SIZE;
				map.tile_id_grid[pos.y][pos.x] = TILE_ID::CLOSED_DOOR;

				map.rendered_entities.push_back(entity);
			}
		}
	}
	
	update_player_sprite();
	clear_and_set_spatial_hash();

	// debugging for memory/component leaks
	registry.list_all_components();
}

void WorldSystem::display_instruction_images()
{
	// WASD
	display_given_instruction(grid_to_world_coord(7, 72), TEXTURE_ASSET_ID::INSTRUCTION_WASD);

	// Left Click
	display_given_instruction(grid_to_world_coord(7, 60), TEXTURE_ASSET_ID::INSTRUCTION_LEFT_CLICK);

	// Space
	display_given_instruction(grid_to_world_coord(7, 36), TEXTURE_ASSET_ID::INSTRUCTION_SPACE); 

	// Dodge
	display_given_instruction(grid_to_world_coord(25, 36), TEXTURE_ASSET_ID::INSTRUCTION_DODGE);

	// R
	display_given_instruction(grid_to_world_coord(7, 62), TEXTURE_ASSET_ID::INSTRUCTION_R);

	// Health Packs
	display_given_instruction(grid_to_world_coord(7, 48), TEXTURE_ASSET_ID::INSTRUCTION_HEALTH_PACK);

	// SHIFT OR F MELEE
	display_given_instruction(grid_to_world_coord(25, 60), TEXTURE_ASSET_ID::INSTRUCTION_MELEE);
}

Entity WorldSystem::display_given_instruction(vec2 position, TEXTURE_ASSET_ID texture_ID)
{
	auto instruction_entity = Entity();
	registry.instructionMessages.emplace(instruction_entity);
	auto& timer = registry.removeTimers.emplace(instruction_entity);
	timer.remaining_time = 1000000;
	auto& motion = registry.motions.emplace(instruction_entity);
	motion.position = position;
	motion.scale = glm::vec2(GRID_CELL_SIZE * 8, GRID_CELL_SIZE * 2);

	registry.textureWithoutLighting.emplace(instruction_entity);
	registry.renderRequests.emplace(instruction_entity, RenderRequest {
			texture_ID,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::NO_LIGHTING,
			true,
		});
	return instruction_entity;
}

// Compute collisions between entities
void WorldSystem::handle_collisions(float elapsed_ms) {

	ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity entity_i = collision_container.entities[i];
		Entity entity_j = collision_container.components[i].other;

		if (registry.pickups.has(entity_i) || registry.pickups.has(entity_j)) {
			Entity pickup = registry.pickups.has(entity_i) ? entity_i : entity_j;
			Entity other = (entity_i == pickup) ? entity_j : entity_i;
			if (registry.players.has(other)) {
				collect_pickup(pickup, audio);
			}
			else {
				Entity projectile = registry.projectiles.has(entity_i) ? entity_i : entity_j;			
				Entity target = (entity_i == projectile) ? entity_j : entity_i;
				handle_projectile_collisions(projectile, target);
			}
		} else if (registry.projectiles.has(entity_i) || registry.projectiles.has(entity_j)) {
			Entity projectile = registry.projectiles.has(entity_i) ? entity_i : entity_j;			
			Entity target = (entity_i == projectile) ? entity_j : entity_i;
			handle_projectile_collisions(projectile, target);
		} else if (registry.staticCollidables.has(entity_i) || registry.staticCollidables.has(entity_j)) {
			Entity wall = registry.staticCollidables.has(entity_i) ? entity_i : entity_j;
			Entity other = (entity_i == wall) ? entity_j : entity_i;
			if (registry.motions.has(other)) {
				handle_wall_collisions(wall, other, elapsed_ms);
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

bool WorldSystem::projectile_hit_door(Motion projectile_motion, vec2 door_loc)
{
	float collision_threshold = 105.0f;
	float distance = glm::distance(projectile_motion.position, door_loc);

	return distance <= collision_threshold;
}

void WorldSystem::handle_projectile_collisions(Entity projectile, Entity target) {
	Projectile& comp = registry.projectiles.get(projectile);
	if (comp.shot_by_player && registry.players.has(target)) {
		return;
	}


	// TODO: REFACTOR THIS TO WORK WITH ALL MAPS
	int current_level = registry.gameProgress.components[0].level;
	Map& map = registry.maps.components[current_level];

	for (auto it = map.prop_doors_list.begin(); it != map.prop_doors_list.end(); ) {
		vec2 door_loc = grid_to_world_coord(it->x, it->y);
		Motion& projectile_motion = registry.motions.get(projectile);

		if (projectile_hit_door(projectile_motion, door_loc)) {
			auto door_it = map.prop_doors.find(*it);
			if (door_it != map.prop_doors.end()) {
				Entity entity = door_it->second;
				Entity entity2;
				for (auto& pair : map.prop_doors_pair) {
					if (pair.first == entity) {
						entity2 = pair.second.back();
						break;
					}
				}

				Prop prop1 = map.props[door_it->first];
				vec2 away_from_player = normalize(door_loc - projectile_motion.position);
				int random_int = (int)uniform_dist(rng) * 10;

				create_debris(door_loc, away_from_player, random_int);
				audio->play_sound(SOUND_ASSET_ID::DOOR_BREAKING, 20);

				// spawn gun pickup in front of doorway
				Motion& target_motion = registry.motions.get(target);
				Projectile& projectile_comp = registry.projectiles.get(projectile);
				vec2 door_normal = get_wall_collision_normal(projectile_motion.position, target_motion.position, target_motion.scale);
				vec2 particle_pos = projectile_motion.position;
				float particle_angle = glm::degrees(atan2(target_motion.position.y, target_motion.position.x) + M_PI / 2.f);
				float particle_scale = 30.f;
				if (projectile_comp.is_gun) {
					Gun& gun = registry.guns.get(projectile);
						Entity entity = spawn_pickup(
							particle_pos + door_normal * GRID_CELL_SIZE * 0.05f,
							projectile_motion.angle,
							PICKUP_TYPE::GUN,
							gun.current_magazine + gun.remaining_bullets,
							gun.gun_type
						);
						registry.guns.insert(entity, gun);
				}

				// change tile id of door from door to floor;
				int entity_id;

				std::vector<ivec2> neighbors = {
					{it->x - 1, it->y - 1},
					{it->x  , it->y - 1},
					{it->x + 1, it->y - 1},
					{it->x - 1, it->y  },
					{it->x  , it->y  },
					{it->x + 1, it->y  },
				};

				for (auto pos : neighbors)
				{
					// Safety check if needed (bounds check)
					if (pos.x < 0 || pos.y < 0 || pos.x >= map.tile_id_grid[0].size() || pos.y >= map.tile_id_grid.size())
						continue;

					// Change tile ID
					map.tile_id_grid[pos.y][pos.x] = TILE_ID::OPEN_DOOR;

					// Update actual tile object
					int entity_id = map.tile_object_grid[pos.y][pos.x];
					if (entity_id == -1) // Skip if no tile object
						continue;

					Tile& tile = map.tiles[entity_id];
					tile.id = TILE_ID::OPEN_DOOR;
				}



				// Remove from all relevant maps
				map.props.erase(*it);
				registry.remove_all_components_of(entity);
				registry.remove_all_components_of(entity2);
				clear_and_set_spatial_hash();
			}

			it = map.prop_doors_list.erase(it);
		}
		else {
			++it;
		}
	}

	if (registry.players.has(target) && !comp.shot_by_player) {
		Player& player = registry.players.get(target);
		if (player.is_invincible) {
			audio->play_sound(SOUND_ASSET_ID::DODGE_WOOSH, 30);
		} else {
			ScreenState& screen = registry.screen_state;
			screen.glitch_remaining_ms = screen.glitch_duration;
			Motion& player_motion = registry.motions.get(target);
			Motion& projectile_motion = registry.motions.get(projectile);
			player_motion.velocity += normalize(player_motion.position - projectile_motion.position) * 100.f;
			// M1: creative element #23: Audio feedback
			// Play groaning sound when user gets hit by a bullet
			player_got_shot(projectile, audio);

			player.health -= comp.damage;
			if (player.health <= 0) {
				audio->play_sound(SOUND_ASSET_ID::PLAYER_HIT_1, 20);
				restart_game();
			}
		}
	}
	else if (registry.enemies.has(target) && comp.shot_by_player) {
		if (comp.hit_enemies.find(target.id()) == comp.hit_enemies.end()) {
			comp.hit_enemies.insert(target.id());
			enemy_got_shot(target, projectile, audio);
			if (comp.remaining_penetrations > 0) {
				comp.remaining_penetrations--;
				return;
			}
		}
		else {
			return;
		}
	}
	else if (registry.pickups.has(target)) {
		return;
	}
	else if (registry.staticCollidables.has(target)) {
		if (comp.ricochet_remaining > 0) {
			audio->play_sound(SOUND_ASSET_ID::BULLET_HIT_WALL_1, 20);
			comp.damage *= 1.5;
			Motion& projectile_motion = registry.motions.get(projectile);
			Motion& wall_motion = registry.motions.get(target);
			vec2 wall_normal = get_wall_collision_normal(projectile_motion.position, wall_motion.position, wall_motion.scale);
			projectile_motion.velocity -= 2.0f * glm::dot(projectile_motion.velocity, wall_normal) * wall_normal;
			projectile_motion.velocity *= 0.8f;
			projectile_motion.position += wall_normal * GRID_CELL_SIZE * 0.05f;
			comp.ricochet_remaining--;
			return;
		}
		else {
			projectile_hit_wall(projectile, target);
		}
	}

	registry.remove_all_components_of(projectile);
}

// M1: creative element #8 Basic Physics
// Prevent objects from moving into each other
void WorldSystem::handle_wall_collisions(Entity wall, Entity other, float elapsed_ms) {
	Motion& wall_motion = registry.motions.get(wall);
	Motion& object_motion = registry.motions.get(other);

	vec2 normal = get_wall_collision_normal(object_motion.position, wall_motion.position, wall_motion.scale);
	float dot_product = dot(object_motion.velocity, normal);
	if (dot_product >= 0) {
		return;
	}

	// for enemies 
	float is_enemy = registry.enemies.has(other);
	// if enemy is in backoff state then check collisions with walls
	if(is_enemy) {
		auto& enemy = registry.enemies.get(other);
		if (enemy.state == ENEMY_STATE::BACKOFF) {
			object_motion.position -= is_enemy * object_motion.velocity * (elapsed_ms / 1000);
		}
	}

	vec2 perpendicular_component = dot_product * normal;
	vec2 parallel_component = object_motion.velocity - perpendicular_component;

	object_motion.velocity = parallel_component;
}

vec2 WorldSystem::get_wall_collision_normal(vec2& player_pos, vec2& wall_pos, vec2& size) {

	float left_wall = wall_pos.x - size.x / 2.f;
	float right_wall = wall_pos.x + size.x / 2.f;
	float bottom_wall = wall_pos.y - size.y / 2.f;
	float top_wall = wall_pos.y + size.y / 2.f;

	float closest_x = max(left_wall, min(player_pos.x, right_wall));
	float closest_y = max(bottom_wall, min(player_pos.y, top_wall));

	vec2 closest_point = vec2(closest_x, closest_y);
	vec2 relative_pos = player_pos - closest_point;

	if (relative_pos.x == 0 && relative_pos.y == 0) {
		return { 0.f, 1.f };
	}

	return normalize(relative_pos);
}

void WorldSystem::projectile_hit_wall(Entity projectile, Entity wall) {
	audio->play_sound(SOUND_ASSET_ID::BULLET_HIT_WALL_1, 20);

	if (!registry.projectiles.has(projectile)) return;

	Motion& projectile_motion = registry.motions.get(projectile);
	Motion& wall_motion = registry.motions.get(wall);
	vec2 wall_normal = get_wall_collision_normal(projectile_motion.position, wall_motion.position, wall_motion.scale);
	vec2 particle_pos = projectile_motion.position;
	float particle_angle = glm::degrees(atan2(wall_normal.y, wall_normal.x) + M_PI / 2.f);
	float particle_scale = 30.f;

	Projectile& projectile_comp = registry.projectiles.get(projectile);
	RenderRequest render_request = registry.renderRequests.get(projectile);

	if (projectile_comp.is_gun) {
		// Reflect and dampen velocity
		projectile_motion.velocity -= 2.0f * glm::dot(projectile_motion.velocity, wall_normal) * wall_normal;
		projectile_motion.velocity *= 0.8f;

		float speed = length(projectile_motion.velocity);
		Gun& gun = registry.guns.get(projectile);

		// Threshold speed to turn into pickup
		if (speed <= 50.f) {
			Entity entity = spawn_pickup(
				particle_pos + wall_normal * GRID_CELL_SIZE * 0.05f,
				projectile_motion.angle,
				PICKUP_TYPE::GUN,
				gun.current_magazine + gun.remaining_bullets,
				gun.gun_type
			);
			registry.guns.insert(entity, gun);
			registry.remove_all_components_of(projectile);
		}
		else if (speed <= 200.f) {
			Entity proj_entity = spawn_pickup(
				particle_pos + wall_normal * GRID_CELL_SIZE * 0.05f,
				projectile_motion.angle,
				PICKUP_TYPE::GUN,
				gun.current_magazine + gun.remaining_bullets,
				gun.gun_type
			);
			Motion motion = registry.motions.get(proj_entity);
			motion.velocity = projectile_motion.velocity;
			registry.guns.insert(proj_entity, gun);
			registry.remove_all_components_of(projectile);
			RenderRequest& render = registry.renderRequests.get(proj_entity);
			render.z_index = Z_INDEX::PICKUP;
			render.used_texture = gun.thrown_sprite;
			// TODO: change sprite size depending on gun
		}
		else {
			Entity proj_entity = createProjectile(
				particle_pos + wall_normal * GRID_CELL_SIZE * 0.05f,
				projectile_motion.scale,
				projectile_motion.velocity,
				projectile_motion.angle,
				projectile_motion.angle_velocity,
				gun.damage * 5,
				true,
				gun.thrown_sprite,
				true
			);
			registry.guns.insert(proj_entity, gun);
			registry.remove_all_components_of(projectile);
		}
	}
	else if (projectile_comp.can_bounce) {
		projectile_motion.velocity -= 2.0f * glm::dot(projectile_motion.velocity, wall_normal) * wall_normal;
		projectile_motion.velocity *= 0.8f;

		Entity proj_entity = createProjectile(
			particle_pos + wall_normal * GRID_CELL_SIZE * 0.05f,
			projectile_motion.scale,
			projectile_motion.velocity,
			projectile_motion.angle,
			projectile_motion.angle_velocity,
			projectile_comp.damage,
			true,
			render_request.used_texture,
			false,
			true
		);
		registry.remove_all_components_of(projectile);
	}
	else {
		create_animation(
			particle_pos,
			{ 0.f, 0.f },
			{ particle_scale, particle_scale },
			particle_angle,
			{
				TEXTURE_ASSET_ID::WALL_PARTICLE_1,
				TEXTURE_ASSET_ID::WALL_PARTICLE_2,
				TEXTURE_ASSET_ID::WALL_PARTICLE_3,
				TEXTURE_ASSET_ID::WALL_PARTICLE_4,
				TEXTURE_ASSET_ID::WALL_PARTICLE_5
			},
			true,
			false,
			30.0f,
			Z_INDEX::WALL_PARTICLE
		);
	}
}

GameState WorldSystem::get_game_state()
{
	return game_state;
}

void WorldSystem::set_game_state(GameState new_game_state)
{
	game_state = new_game_state;
}

