#include "player_system.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "input_system.hpp"
#include "world_init.hpp"
#include "map_init.hpp"
#include "animation_init.hpp"
#include "common.hpp"
#include "decision_tree_ai.hpp"
#include <SDL.h>
#include <cmath>
#include <iostream>
#include <glm/trigonometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <ai_system_init.hpp>
#include <physics_system_init.hpp>

bool PlayerSystem::step(float elapsed_ms) {
	if (registry.players.entities.empty()) {
		return false;
	}

	if (world->get_game_state() == GameState::TITLE_SCREEN || world->get_game_state() == GameState::CINEMATIC) {
		update_crosshair_position();
		return false;
	}
	progress_timers(elapsed_ms);
	updatePlayerRotation();
	handle_dash(elapsed_ms);
	update_player_velocity(elapsed_ms);
	update_camera_position(elapsed_ms);
	update_crosshair_position();
	handle_melee();
	if (registry.guns.has(player)) {
		handleShooting();
		handle_reload();
	}
	handle_exit();
 
	return true;
}

void PlayerSystem::updatePlayerRotation() {
	auto& player_motion = registry.motions.get(player);
	vec2 player_dcs_pos = this->renderer->wcs_to_dcs(player_motion.position);

	auto& inputs = registry.inputs.get(registry.inputs.entities[0]);
	vec2 mouse_dir = inputs.mouse_pos - player_dcs_pos;
	float angle = glm::degrees(atan2(mouse_dir.y, mouse_dir.x) + M_PI);
	player_motion.angle = angle;

	// Update laser
	auto& laser_motion = registry.motions.get(laser);
	laser_motion.angle = angle + 180;
	laser_motion.position = get_laser_position();
}

void PlayerSystem::init(RenderSystem* renderer, AudioSystem* audio, WorldSystem* world, GLFWwindow* window) {
	this->renderer = renderer;
	this->audio = audio;
	this->world = world;
	this->window = window;

	Entity player_entity = createPlayer(grid_to_world_coord(16, 78));
	create_laser();
	create_crosshair();
	create_gun(player_entity, GUN_TYPE::PISTOL);
	registry.walkSoundTimers.emplace(player_entity);
}

Entity PlayerSystem::createPlayer(vec2 position) {
	if (!registry.players.entities.empty()) {
		registry.players.clear();
	}

	Entity entity = Entity();
	Player& player = registry.players.emplace(entity);
	player.speed = GRID_CELL_SIZE * 5;
	player.health = STARTING_PLAYER_HEALTH / 2;

	this->player = entity;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& playerMotion = registry.motions.emplace(entity);
	playerMotion.position = position;
	playerMotion.angle = 0.0f;
	playerMotion.velocity = glm::vec2(0.0f, 0.0f);
	playerMotion.scale = glm::vec2(GRID_CELL_SIZE * 2.8, GRID_CELL_SIZE * 2.8);

	registry.movingCollidables.emplace(entity);
	registry.movingCircleCollidables.emplace(entity);
	Collidable& collidable = registry.collidables.emplace(entity);
	CircleBound& circle_bound = registry.circlebounds.emplace(entity);
	circle_bound.collision_radius = GRID_CELL_SIZE / 2.;
	circle_bound.offset = { 0.f, 0.f };

	Dash& dash = registry.dashes.emplace(entity);
	dash.dash_speed = GRID_CELL_SIZE * 17;

	Melee& melee = registry.melees.emplace(entity);

	RenderRequest& render = registry.renderRequests.emplace(entity, RenderRequest{
		TEXTURE_ASSET_ID::PLAYER_SHOOTING_PISTOL,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::PLAYER,
		false
	});
	
	render.outline_color_a = vec3(0.0, 0.75, 0.65);
	render.outline_color_b = vec3(0.0, 1.0, 0.9);

	 Entity camera_entity = registry.cameras.entities[0];
	 Motion& camera_motion = registry.motions.get(camera_entity);
	 camera_motion.position = playerMotion.position;

	return entity;
}

Entity PlayerSystem::create_laser() {
	if (!registry.lasers.entities.empty()) {
		registry.lasers.clear();
	}

	Entity entity = Entity();
	Laser& laser = registry.lasers.emplace(entity);

	this->laser = entity;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& playerMotion = registry.motions.emplace(entity);
	playerMotion.position = get_laser_position();
	playerMotion.angle = 0.0f;
	playerMotion.velocity = glm::vec2(0.0f, 0.0f);
	playerMotion.scale = glm::vec2(200.0f, 200.0f);

	registry.renderRequests.emplace(entity, RenderRequest{
		TEXTURE_ASSET_ID::LASER,
		EFFECT_ASSET_ID::TEXTURED, 
		GEOMETRY_BUFFER_ID::SPRITE,
    	Z_INDEX::LASER
	});
	return entity;
}

void PlayerSystem::create_crosshair() {
	Entity entity = Entity();
	this->crosshair = entity;

	auto& crosshair_motion = registry.motions.emplace(entity);
	crosshair_motion.position = {200, 200};
	crosshair_motion.scale = glm::vec2(20.0f, 20.0f);

	registry.uis.emplace(entity);
	registry.renderRequests.emplace(entity, RenderRequest{
		TEXTURE_ASSET_ID::CROSSHAIR,
		EFFECT_ASSET_ID::TEXTURED, 
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::CROSSHAIR,
		true
	});
}

void PlayerSystem::progress_timers(float elapsed_ms) {
	if (registry.guns.has(this->player)) {
		auto& gun = registry.guns.get(this->player);
		if (gun.cooldown_timer_ms > 0) {
			gun.cooldown_timer_ms = gun.cooldown_timer_ms - elapsed_ms;
		}

		if (gun.throw_timer_ms > 0) {
			gun.throw_timer_ms -= elapsed_ms;
		}
	}
	
	auto& walkSoundTimer = registry.walkSoundTimers.get(this->player);
	if (walkSoundTimer.counter_ms > 0) {
		walkSoundTimer.counter_ms = max(0.0f, walkSoundTimer.counter_ms - elapsed_ms);
	}

	auto& meleeTimer = registry.melees.get(this->player);
	if (meleeTimer.counter_ms > 0) {
		meleeTimer.counter_ms = max(0.0f, meleeTimer.counter_ms - elapsed_ms);
	}

	auto& dash_timer = registry.dashes.get(this->player);
	auto& player_component = registry.players.get(this->player);
	// Reduce invincibility timer
	if (dash_timer.invincibility_timer_ms > 0.0f) {
		dash_timer.invincibility_timer_ms = dash_timer.invincibility_timer_ms - elapsed_ms;
	}
	else {
		player_component.is_invincible = false;
	}

	// Reduce cooldown timer
	if (dash_timer.cooldown_timer > 0.0f) {
		dash_timer.cooldown_timer = dash_timer.cooldown_timer - elapsed_ms;
	}

	if (dash_timer.ghost_timer_ms > 0.f) {
		dash_timer.ghost_timer_ms -= elapsed_ms;
	}

	if (dash_timer.ghost_spawn_ms > 0.f) {
		dash_timer.ghost_spawn_ms -= elapsed_ms;
	}


	if (registry.reloads.has(player)) {
		Reload& reload = registry.reloads.get(player);
		reload.remaining_time -= elapsed_ms;
	}
}

void PlayerSystem::handle_melee() {
	auto& input = registry.inputs.get(registry.inputs.entities[0]);

	// Shoot projectile if left button is pressed
	if (!input.keys[GLFW_KEY_F] && !input.keys[GLFW_KEY_LEFT_SHIFT]) {
		return;
	}

	Melee& melee = registry.melees.get(player);
	if (melee.counter_ms <= melee.melee_cooldown_ms - melee.melee_hit_ms) {
		melee.is_melee = false;
	}

	if (melee.counter_ms <= 0.f) {
		melee.melee_cooldown = false;
	}

	if (melee.melee_cooldown) {
		return;
	}

	// Activate melee attack
	melee.is_melee = true;
	melee.melee_cooldown = true;
	melee.counter_ms = melee.melee_cooldown_ms;

	Motion& motion = registry.motions.get(player);
	audio->play_sound(SOUND_ASSET_ID::SLASH, 40);

	// Melee Animation
	create_animation(
		motion.position, motion.velocity, { 300.0f, 200.0f },
		motion.angle + 180,
		std::vector<TEXTURE_ASSET_ID>{TEXTURE_ASSET_ID::SLASH_0, TEXTURE_ASSET_ID::SLASH_1, TEXTURE_ASSET_ID::SLASH_2, TEXTURE_ASSET_ID::SLASH_3},
		true, false, 50.0f, Z_INDEX::MUZZLE_FLASH);

	// Melee Hit Detection
	float melee_radius = melee.melee_range;
	float melee_half_angle = glm::radians(100.f);


	// code to check if projectiles hit door. Refactor to check if melee hits doors
	int current_level = registry.gameProgress.components[0].level;
	Map& map = registry.maps.components[current_level];

	for (auto it = map.prop_doors_list.begin(); it != map.prop_doors_list.end(); ) {
		vec2 door_loc = grid_to_world_coord(it->x, it->y);

		// Vector from player to door
		vec2 delta_pos = door_loc - motion.position;
		float distance = glm::length(delta_pos);

		// Check if door is within melee range
		if (distance > melee_radius) {
			++it;
			continue;
		}

		// Check if door is within melee angle
		vec2 player_dir = { -cos(glm::radians(motion.angle)), -sin(glm::radians(motion.angle)) };
		vec2 door_dir = glm::normalize(delta_pos);

		float door_angle = glm::orientedAngle(door_dir, player_dir);
		if (std::abs(door_angle) > melee_half_angle) {
			++it;
			continue;
		}

		// Door is hit by melee
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

			vec2 away_from_player = normalize(door_loc - motion.position);
			int random_int = get_rand(0, 10);
			create_debris(door_loc, away_from_player, random_int);

			audio->play_sound(SOUND_ASSET_ID::DOOR_BREAKING, 20);

			// Remove from all relevant maps
			map.props.erase(*it);
			registry.remove_all_components_of(entity);
			registry.remove_all_components_of(entity2);
			clear_and_set_spatial_hash();
		}

		it = map.prop_doors_list.erase(it);
	}

	for (Entity enemy : registry.enemies.entities) {
		Motion& enemy_motion = registry.motions.get(enemy);

		// Vector from player to enemy
		vec2 delta_pos = enemy_motion.position - motion.position;
		float distance = glm::length(delta_pos);

		// Check if enemy is within melee range
		if (distance > melee_radius) continue;
		vec2 player_dir = { -cos(glm::radians(motion.angle)), -sin(glm::radians(motion.angle)) };
		vec2 enemy_dir = glm::normalize(delta_pos);

		// Compute signed angle between player and enemy
		float enemy_angle = glm::orientedAngle(enemy_dir, player_dir);
		vec2 delta_position = enemy_motion.position - motion.position;
		float delta_difference = 30.f;

		// Check if enemy is within the circular arc
		if (std::abs(enemy_angle) <= melee_half_angle || std::abs(delta_position.x) <= delta_difference && std::abs(delta_position.y) <= delta_difference) {
			//std::cout << "MELEE HIT!" << std::endl;

			auto& enemy_comp = registry.enemies.get(enemy);

			enemy_comp.health -= melee.melee_damage;

			if (enemy_comp.health <= 0) {
				SOUND_ASSET_ID random_enemy_hit_sound = static_cast<SOUND_ASSET_ID>(get_rand(static_cast<int>(SOUND_ASSET_ID::ENEMY_HIT_1), static_cast<int>(SOUND_ASSET_ID::ENEMY_HIT_8)));
				audio->play_sound(random_enemy_hit_sound, 5);
				registry.guns.remove(enemy);
				create_dead_enemy(enemy_motion.position, enemy_motion.angle);
				spawn_pickup(enemy_motion.position, 0, PICKUP_TYPE::GUN, 10, GUN_TYPE::PISTOL);

				registry.remove_all_components_of(enemy);

				if (registry.enemies.size() - registry.tutorialEnemies.size() <= 0) {
					// TODO: Why do we do this check in two places
					std::cout << "Level cleared" << std::endl;
					registry.map_system->load_next_map();
				}
			}
			else {
				enemy_motion.velocity += normalize(enemy_motion.position - motion.position) * 100.f;
			}

			create_animation(
				enemy_motion.position,
				enemy_motion.velocity,
				{ 150.f, 150.f },
				motion.angle + 90.f,
				std::vector<TEXTURE_ASSET_ID>{
				TEXTURE_ASSET_ID::BLOOD_SPLATTER_0,
					TEXTURE_ASSET_ID::BLOOD_SPLATTER_1,
					TEXTURE_ASSET_ID::BLOOD_SPLATTER_2,
					TEXTURE_ASSET_ID::BLOOD_SPLATTER_3,
					TEXTURE_ASSET_ID::BLOOD_SPLATTER_4,
					TEXTURE_ASSET_ID::BLOOD_SPLATTER_5,
					TEXTURE_ASSET_ID::BLOOD_SPLATTER_6,
					TEXTURE_ASSET_ID::BLOOD_SPLATTER_7,
			},
				true,
				false,
				50.0f,
				Z_INDEX::BLOOD_SPLATTER
			);
		}
	}
}

void PlayerSystem::handleShooting() {
	auto& input = registry.inputs.get(registry.inputs.entities[0]);

	// Shoot projectile if left button is pressed
	if (!input.keys[GLFW_MOUSE_BUTTON_LEFT] && !input.key_down_events[GLFW_MOUSE_BUTTON_RIGHT]) {
		return;
	}

	if (!registry.guns.has(player)) {
		return;
	}

	auto& gun = registry.guns.get(player);

	if (input.key_down_events[GLFW_MOUSE_BUTTON_RIGHT]) {
		if (gun.throw_timer_ms > 0.f) {
			return;
		}
		auto& player_motion = registry.motions.get(player);
		vec2 player_dcs_pos = this->renderer->wcs_to_dcs(player_motion.position);
		vec2 mouse_dir = normalize(input.mouse_pos - player_dcs_pos);
		vec2 projectile_velocity = mouse_dir * 1000.f;
		vec2 gun_position = get_gun_position();
		Entity projectile_entity = createProjectile(gun_position, { GRID_CELL_SIZE * 0.75, GRID_CELL_SIZE * 0.75 }, projectile_velocity, player_motion.angle + 180.0f, 2.f, 
			gun.damage * 5, true, gun.thrown_sprite, true);
		Gun& new_gun = registry.guns.insert(projectile_entity, gun);
		new_gun.throw_timer_ms = new_gun.throw_cooldown_time;
		registry.guns.remove(player);
		registry.ui_system->update_gun_ui();
		return;
	}

	if (gun.cooldown_timer_ms > 0) {
		return;
	}

	if (registry.reloads.has(player)) {
		return;
	}

	gun.cooldown_timer_ms = gun.cooldown_ms;

	if (gun.current_magazine == 0) {
		audio->play_sound(gun.click_effect, 50);
		registry.inputs.components[0].key_down_events[GLFW_KEY_R] = true;
		return;
	}

	gun.current_magazine--;
	auto& player_motion = registry.motions.get(player);
	vec2 player_dcs_pos = this->renderer->wcs_to_dcs(player_motion.position);
	vec2 mouse_dir = normalize(input.mouse_pos - player_dcs_pos);
	vec2 projectile_velocity = mouse_dir * gun.projectile_speed;
	vec2 gun_position = get_gun_position();

	for (int i = 0; i < gun.projectile_count; i++) {
		float spread_angle = 0.f;
		
		if (gun.projectile_count > 1) {
			float angle_step = gun.spread_cone / (gun.projectile_count - 1);
			spread_angle = -gun.spread_cone / 2.f + i * angle_step;
		}
		
		float cos_spread = cos(radians(spread_angle));
		float sin_spread = sin(radians(spread_angle));
		
		vec2 projectile_dir = vec2(
			mouse_dir.x * cos_spread - mouse_dir.y * sin_spread,
			mouse_dir.x * sin_spread + mouse_dir.y * cos_spread
		);
		
		vec2 projectile_velocity = projectile_dir * gun.projectile_speed;
		
		createProjectile(
			gun_position, 
			{ 15, 15 }, 
			projectile_velocity, 
			player_motion.angle + 180.0f + spread_angle,
			0.f, 
			gun.damage, 
			true, 
			TEXTURE_ASSET_ID::PROJECTILE,
			false,
			false,
			gun.penetrating_count,
			gun.ricochet_count
		);
	}
	// M1: creative element #23: Audio feedback
	// Play gunshot sound when user shoots projectile  
	audio->play_sound(gun.sound_effect, 15);
	create_animation(gun_position, player_motion.velocity, {20.0f, 20.0f}, player_motion.angle + 180, std::vector<TEXTURE_ASSET_ID>{TEXTURE_ASSET_ID::MUZZLE_FLASH0, TEXTURE_ASSET_ID::MUZZLE_FLASH1, TEXTURE_ASSET_ID::MUZZLE_FLASH2},
					 true, false, 50.0f, Z_INDEX::MUZZLE_FLASH);
    add_temp_light(gun_position, {1.0, 0.6, 0.2}, 200, 1.f, true, 100.f);

	player_motion.velocity -= mouse_dir * GRID_CELL_SIZE * gun.recoil_pushback;

	// alert all enemies in the same room as the player
	for (Entity& enemy : registry.enemies.entities) {
		if (is_enemy_in_same_room_as_player(enemy)) {
			registry.enemies.get(enemy).state = ENEMY_STATE::PURSUIT;
			// invalidate the cached path so a new path is computed
			if (registry.pathComponents.has(enemy)) {
				auto& pathComp = registry.pathComponents.get(enemy);
				pathComp.valid = false;
			}
		}
	}
}

// handles dashing mechanics
void PlayerSystem::handle_dash(float elapsed_ms) {
	auto& player_component = registry.players.get(player);
	auto& player_motion = registry.motions.get(player);
	auto& player_inputs = registry.inputs.get(registry.inputs.entities[0]);
	auto& player_dash = registry.dashes.get(player);
	auto& player_render = registry.renderRequests.get(player);

	float target_transparency = player_dash.invincibility_timer_ms > 0. ? 1.0f : 0.0f;
	if (player_render.outline_transparency != target_transparency) {
		float transition_speed = 1.0f / SHIELD_TRANSITION_TIME_MS;
		float step = elapsed_ms * transition_speed;
		
		if (target_transparency > player_render.outline_transparency) {
			player_render.outline_transparency = min(target_transparency, player_render.outline_transparency + step);
		} else {
			player_render.outline_transparency = max(target_transparency, player_render.outline_transparency - step);
		}
	}

	if (player_dash.is_dashing) {
		// Compute dash progress based on actual distance moved
		float dash_step = length(player_dash.dash_velocity) * (elapsed_ms / 1000.0f);
		player_dash.curr_distance += dash_step;

		// Stop dashing if distance covered
		if (player_dash.curr_distance >= player_dash.dash_distance) {
			player_dash.dash_velocity = vec2(0.0f, 0.0f);
			player_dash.is_dashing = false;
			return;
		}

		// Continue dashing
		player_motion.velocity = player_dash.dash_velocity;
		return;
	}

	if (player_dash.ghost_timer_ms > 0.f && player_dash.ghost_spawn_ms <= 0.f) {
		float time_factor = player_dash.ghost_timer_ms / player_dash.ghost_duration;
		Entity ghost_entity = Entity();
		Motion& ghost_motion = registry.motions.insert(ghost_entity, player_motion);
		ghost_motion.velocity = { 0.f, 0.f };
		RenderRequest& ghost_render = registry.renderRequests.insert(ghost_entity, player_render);
		ghost_render.outline_transparency = 0.;
		float alpha = mix(0.3f, 0.1f, time_factor);
		ghost_render.alpha = alpha;
		ghost_render.z_index = Z_INDEX::PLAYER_DASH;
		RemoveTimer& timer = registry.removeTimers.emplace(ghost_entity);
		
		const vec3 colors[] = {
			vec3(0.0f, 1.0f, 0.624f),
			vec3(0.0f, 0.722f, 1.0f),
			vec3(0.0f, 0.118f, 1.0f),
			vec3(0.741f, 0.0f, 1.0f),
			vec3(0.839f, 0.0f, 1.0f)
		};

		float color_pos = time_factor * colors->length();
		int color_i = min(5.f, floor(color_pos));
		int color_j = min(5.f, color_i + 1.f);
		float mix_factor = color_pos - color_i;
		vec3 color = mix(colors[color_i], colors[color_j], mix_factor);
		registry.colors.insert(ghost_entity, color);
		timer.remaining_time = DASH_GHOST_DURATION_TIMER_MS;

		player_dash.ghost_spawn_ms = DASH_GHOST_TIMER_MS;
	}

	// Prevent dashing if cooldown is still active
	if (player_dash.cooldown_timer > 0.0f) {
		return;
	}

	// Start dash if space is pressed and not already dashing
	if (player_inputs.keys[GLFW_KEY_SPACE] && !player_dash.is_dashing) {
		audio->play_sound(SOUND_ASSET_ID::DASH, 15);
		player_component.is_invincible = true;
		player_dash.is_dashing = true;
		// Start both invincibility and cooldown timers at the same time
		player_dash.invincibility_timer_ms = player_dash.invincibility_duration;
		player_dash.cooldown_timer = player_dash.dash_cooldown_ms;
		player_dash.curr_distance = 0.0f;
		player_dash.ghost_timer_ms = player_dash.ghost_duration;

		// Normalize current velocity and apply dash speed
		if (length(player_motion.velocity) > 10.0f) {
			player_dash.dash_velocity = normalize(player_motion.velocity) * player_dash.dash_speed;
		}
		else {
			// Dash towards the mouse if player is not moving
			float angle = glm::radians(player_motion.angle);
			player_dash.dash_velocity = {
				-cos(angle) * player_dash.dash_speed,
				-sin(angle) * player_dash.dash_speed
			};
		}

		player_motion.velocity = player_dash.dash_velocity;
	}
}

void PlayerSystem::handle_reload() {
	auto& player_inputs = registry.inputs.components[0];
	if (!registry.guns.has(player)) {
		return;
	}
	auto& player_gun = registry.guns.get(player);
	if (registry.reloads.has(player)) {
		Reload& reload = registry.reloads.get(player);
		if (reload.remaining_time <= 0.f) {
			if (reload.one_at_a_time) {
				if (player_gun.current_magazine < player_gun.magazine_size && player_gun.remaining_bullets > 0) {
					player_gun.current_magazine += 1;
					player_gun.remaining_bullets -= 1;
					
					if (player_gun.current_magazine < player_gun.magazine_size && player_gun.remaining_bullets > 0) {
						reload.remaining_time = player_gun.reload_time;
						audio->play_sound(player_gun.reload_effect, 50);
					} else {
						audio->play_sound(player_gun.rack_effect, 50);
						registry.reloads.remove(player);
					}
				} else {
					audio->play_sound(player_gun.rack_effect, 50);
					registry.reloads.remove(player);
				}
			} else {
				int amount_to_reload = player_gun.magazine_size - player_gun.current_magazine;
				int bullets_to_transfer = min(amount_to_reload, player_gun.remaining_bullets);
				player_gun.current_magazine += bullets_to_transfer;
				player_gun.remaining_bullets -= bullets_to_transfer;
				registry.reloads.remove(player);
			}
		}
		return;
	}

	if (!player_inputs.key_down_events[GLFW_KEY_R]) {
		return;
	}

	Reload& reload = registry.reloads.emplace(player);
	reload.remaining_time = player_gun.reload_time;
	reload.one_at_a_time = player_gun.reload_one_at_a_time;
	audio->play_sound(player_gun.reload_effect, 50);
}


// changes player velocity to direction of input
void PlayerSystem::update_player_velocity(float elapsed_ms) {
	auto& playerMotion = registry.motions.get(player);
	auto& playerInputs = registry.inputs.get(registry.inputs.entities[0]);
	auto& playerComponent = registry.players.get(player);
	float speed = playerComponent.speed;
	
	// reset velocity
	vec2 targetVelocity = vec2(0.f, 0.f);

	// check keys and set velocity as needed
	float is_pressing_w = (playerInputs.keys[GLFW_KEY_W]);
	float is_pressing_s = (playerInputs.keys[GLFW_KEY_S]);
	float is_pressing_a = (playerInputs.keys[GLFW_KEY_A]);
	float is_pressing_d = (playerInputs.keys[GLFW_KEY_D]);
	targetVelocity.y -= is_pressing_w * speed;
	targetVelocity.y += is_pressing_s * speed;
	targetVelocity.x -= is_pressing_a * speed;
	targetVelocity.x += is_pressing_d * speed;

	float velocity_length = length(targetVelocity);
	float epsilon = 1e-5;
	targetVelocity = (targetVelocity / (velocity_length + epsilon)) * speed;

	if (length(targetVelocity) != 0) {
		auto& walkSoundTimer = registry.walkSoundTimers.get(this->player);
		if (walkSoundTimer.counter_ms == 0) {
			// M1: creative element #23: Audio feedback
			// Play footstep sound when user walks around
			audio->play_sound(SOUND_ASSET_ID::FOOTSTEP, 80);
			walkSoundTimer.counter_ms = WALK_SOUND_TIMER_MS;
		}
	}

	// M1 interpolation implementation, smooths player velocity so it's not as "snappy", and feels like player is accelerating to final velocity
	// M1: creative element #8 Basic Physics
	// Adds a friction constant
	float smooth_constant = 10.f;
	float time_adjusted_constant = 1.0f - expf(-smooth_constant * (elapsed_ms / 1000.f));
	playerMotion.velocity = lerp(playerMotion.velocity, targetVelocity, time_adjusted_constant);
}

void PlayerSystem::update_camera_position(float elapsed_ms) {
	Entity camera_entity = registry.cameras.entities[0];
	Input& input = registry.inputs.components[0];
	Camera& camera = registry.cameras.components[0];
	Motion& player_motion = registry.motions.get(player);
	Motion& camera_motion = registry.motions.get(camera_entity);

	vec2 mouse_wc = renderer->dcs_to_wcs(camera_motion, input.mouse_pos);
	vec2 player_to_mouse_vector = mouse_wc - player_motion.position;
	float player_mouse_distance = length(player_to_mouse_vector);

	float dampening_factor = (player_mouse_distance - camera.deadzone) / (camera.max_distance - camera.deadzone);
	dampening_factor = clamp(dampening_factor, 0.f, 1.f);

	float is_mouse_outside_deadzone = (player_mouse_distance >= camera.deadzone);
	vec2 target_position = player_motion.position;
	float epsilon = 1e-5;
	target_position += (player_to_mouse_vector / (player_mouse_distance + epsilon)) * dampening_factor * camera.dampening_offset;

	camera_motion.position = target_position;

	// TODO: Re-add once we fix, or completely remove if we like current camera better
	//vec2 dir_to_target = target_position - camera_motion.position;
	//float dir_length = length(dir_to_target);
	//float epsilon = 1e-5f;
	//vec2 target_velocity = (dir_to_target / (dir_length + epsilon)) * camera.speed;
	//camera_motion.velocity = target_velocity;
}

void PlayerSystem::update_crosshair_position() {
	Motion& crosshair_motion = registry.motions.get(crosshair);
	auto& inputs = registry.inputs.get(registry.inputs.entities[0]);

	crosshair_motion.position = inputs.mouse_pos;
}

vec2 PlayerSystem::get_gun_position() {
	auto& player_motion = registry.motions.get(player);
	vec2 gun_offset = vec2(-player_motion.scale.x/2, 0);
	float rad = glm::radians(player_motion.angle);
	vec2 rotated_offset = {
		gun_offset.x * cos(rad) - gun_offset.y * sin(rad),
		gun_offset.x * sin(rad) + gun_offset.y * cos(rad)
	};
	return player_motion.position + rotated_offset;
}

vec2 PlayerSystem::get_laser_position() {
	auto& player_motion = registry.motions.get(player);
	float laser_size = 200;
	vec2 laser_offset = vec2(-(player_motion.scale.x/2 + laser_size/2), 0);
	float rad = glm::radians(player_motion.angle);
	vec2 rotated_offset = {
		laser_offset.x * cos(rad) - laser_offset.y * sin(rad),
		laser_offset.x * sin(rad) + laser_offset.y * cos(rad)
	};
	return player_motion.position + rotated_offset;
}

void player_got_shot(Entity projectile, AudioSystem* audio) {
	audio->play_sound(SOUND_ASSET_ID::BULLET_HIT_FLESH_1, 25);

	Entity player = registry.players.entities[0];
	Motion& projectile_motion = registry.motions.get(projectile);
	Motion& player_motion = registry.motions.get(player);

	create_animation(
		player_motion.position, 
		{ 0.f, 0.f },
		{ 150.f, 150.f },
		projectile_motion.angle + 270.f,
		std::vector<TEXTURE_ASSET_ID>{
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_0,
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_1,
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_2,
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_3,
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_4,
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_5,
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_6,
			TEXTURE_ASSET_ID::BLOOD_SPLATTER_7,
		},
		true,
		false,
		30.0f,
		Z_INDEX::BLOOD_SPLATTER
	);

	create_animation(
		player_motion.position,
		{ 0.f, 0.f },
		{ 300.f, 300.f },
		projectile_motion.angle + 90.f,
		std::vector<TEXTURE_ASSET_ID>{
			TEXTURE_ASSET_ID::DAMAGE_INDICATOR_0,
			TEXTURE_ASSET_ID::DAMAGE_INDICATOR_1,
			TEXTURE_ASSET_ID::DAMAGE_INDICATOR_2,
			TEXTURE_ASSET_ID::DAMAGE_INDICATOR_3,
		},
		true,
		false,
		50.0f,
		Z_INDEX::BLOOD_SPLATTER,
		false,
		true
	);
}

void PlayerSystem::handle_exit() {
	auto& input = registry.inputs.get(registry.inputs.entities[0]);

	if (!input.keys[GLFW_KEY_ESCAPE]) {
		return;
	}

	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void update_player_sprite() {
	if (!registry.players.entities.empty()) {
		Entity& player_entity = registry.players.entities[0];
		Gun& player_gun = registry.guns.get(player_entity);
		RenderRequest& player_render_request = registry.renderRequests.get(player_entity);
		if (player_gun.gun_type == GUN_TYPE::SHOTGUN) {
			player_render_request.used_texture = TEXTURE_ASSET_ID::PLAYER_SHOOTING_SHOTGUN;
		} else {
			player_render_request.used_texture = TEXTURE_ASSET_ID::PLAYER_SHOOTING_PISTOL;
		}
	}
}
