#include <iostream>
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "ai_system.hpp"
#include "ai_system_init.hpp"
#include "world_init.hpp"
#include "map_init.hpp"
#include <SDL.h>
#include <cmath>
#include <string>
#include <glm/trigonometric.hpp>
#include <animation_init.hpp>

#include "decision_tree_ai.hpp"
#include "a_star_pathfinding.hpp"

void AISystem::step(float elapsed_ms)
{
	if (!decisionTree)
		decisionTree = build_decision_tree();

	for (const Entity& enemy_entity : registry.enemies.entities) {
		progress_timers(elapsed_ms, enemy_entity);

		// Evaluate the decision tree for this enemy.
		ENEMY_ACTION action = decisionTree->evaluate(enemy_entity);
		auto& enemy = registry.enemies.get(enemy_entity);
		auto& enemyMotion = registry.motions.get(enemy_entity);

		// if no player exists ex. player dies
		if (registry.players.entities.empty()) {
			// if no player, do nothing
			enemyMotion.velocity = { 0, 0 };
			enemy.state = ENEMY_STATE::IDLE;
			continue;
		}

		// execute behavior based on the decision tree output
		switch (action) {
		case ENEMY_ACTION::ACTION_IDLE:
			//std::cout << "IDLE" << std::endl;

			// reset begin_shooting_timer so that when enemy goes back into combat, they won't shoot immediately
			enemy.begin_shooting_ms = enemy.begin_shooting_timer;

			if (enemy.stop_cooldown_ms > 0.f) {
				enemy.stop_cooldown_ms = enemy.stop_cooldown_ms - elapsed_ms;
			}
			else {
				enemyMotion.velocity = { 0, 0 };
			}
			enemy.state = ENEMY_STATE::IDLE;
			break;
		case ENEMY_ACTION::ACTION_APPROACH:
			//std::cout << "APPROACH" << std::endl;

			update_enemy_rotation(enemy_entity, elapsed_ms);
			move_toward_player(enemy_entity);
			enemy.state = ENEMY_STATE::APPROACH;
			break;
		case ENEMY_ACTION::ACTION_COMBAT:
			//std::cout << "COMBAT" << std::endl;

			enemyMotion.velocity = { 0, 0 };
			update_enemy_rotation(enemy_entity, elapsed_ms);
			enemy.state = ENEMY_STATE::COMBAT;
			if (enemy.begin_shooting_ms > 0.f) {
				enemy.begin_shooting_ms = enemy.begin_shooting_ms - elapsed_ms;
			}
			else {
				shoot_if_aggro(enemy_entity, elapsed_ms);
			}
			break;
		case ENEMY_ACTION::ACTION_BACKOFF:
			//std::cout << "BACKOFF" << std::endl;

			enemy.state = ENEMY_STATE::BACKOFF;
			update_enemy_rotation(enemy_entity, elapsed_ms);
			move_away_from_player(enemy_entity);
			shoot_if_aggro(enemy_entity, elapsed_ms);
			break;
		case ENEMY_ACTION::ACTION_PURSUIT:
			//std::cout << "PURSUIT" << std::endl;

			enemy.state = ENEMY_STATE::PURSUIT;

			update_enemy_rotation(enemy_entity, elapsed_ms);

			Entity player = registry.players.entities[0];
			Motion& player_motion = registry.motions.get(player);

			ivec2 enemy_cell = world_to_grid_coords(enemyMotion.position.x, enemyMotion.position.y);
			ivec2 player_cell = world_to_grid_coords(player_motion.position.x, player_motion.position.y);

			// get or add the PathComponent assuming they dont already have one
			if (!registry.pathComponents.has(enemy_entity)) {
				registry.pathComponents.emplace(enemy_entity);
			}
			auto& pathComp = registry.pathComponents.get(enemy_entity);

			// if enemy loses line of sight and player is moving, then invalidate the path
			// reset stop timer so that when enemy reaches path, they don't stop immediately on the corner
			// but rather move past the corner slightly
			if (enemy_has_los_to_player(enemy_entity) && pathComp.target_cell != player_cell) {
				pathComp.valid = false;
				enemy.stop_cooldown_ms = enemy.stop_timer;
			}

			// majority of path calculation here
			if (!pathComp.valid) {
				int current_level = registry.gameProgress.components[0].level;
				Map& map = registry.maps.components[current_level];
				std::vector<ivec2> newPath = find_path(enemy_cell, player_cell, map);
				if (newPath.empty()) {
					// no path found
					enemyMotion.velocity = { 0, 0 };
					break;
				}
				// store the path once found
				pathComp.waypoints = newPath;
				pathComp.current_index = 0;
				pathComp.valid = true;
				pathComp.target_cell = player_cell;
			}

			update_enemy_rotation(enemy_entity, elapsed_ms);

			// follow the precomputed path
			if (pathComp.current_index < pathComp.waypoints.size()) {
				ivec2 gridPos = pathComp.waypoints[pathComp.current_index];
				vec2 waypoint_world = grid_to_world_coord(gridPos.x, gridPos.y);

				// check if distance is large enough to avoid zero-length direction???
				vec2 delta = waypoint_world - enemyMotion.position;
				float dist = length(delta);
				if (dist > 30.f) {
					vec2 direction = delta / dist;
					enemyMotion.velocity = direction * enemy.speed;
				}
				else {
					// close enough to this waypoint so move to next
					pathComp.current_index++;
				}
			}
			else {
				// no more waypoints
				enemyMotion.velocity = { 0,0 };
			}
			break;
		}
	}
	
}

void AISystem::init(RenderSystem* renderer, AudioSystem* audio) {
	this->renderer = renderer;
	this->audio = audio;

	decisionTree = build_decision_tree();  // Ensure decision tree is built.
}

void AISystem::update_velocity(Entity entity, float elapsed_ms) {
	Enemy& enemy = registry.enemies.get(entity);
	Motion& motion = registry.motions.get(entity);
	if (enemy.state == ENEMY_STATE::IDLE) {
		float smooth_constant = 10.f;
		float time_adjusted_constant = 1.0f - expf(-smooth_constant * (elapsed_ms / 1000.f));
		motion.velocity = lerp(motion.velocity, {0., 0.}, time_adjusted_constant);
	}
}

// gets the distance from the enemy entity to the player
float AISystem::get_distance_to_player(Entity entity) {

	Entity player = registry.players.entities[0];
	auto& player_motion = registry.motions.get(player);
	auto& enemy_motion = registry.motions.get(entity);

	// Euclidean distance
	return length(player_motion.position - enemy_motion.position);
}

// checks if enemy is currently set to combat state, and if so then enemy rotates to player and shoots
void AISystem::shoot_if_aggro(Entity entity, float elapsed_ms) {
	auto& enemy = registry.enemies.get(entity);

	if (registry.players.entities.empty()) {
		return;
	}

	update_enemy_rotation(entity, elapsed_ms);

	if (!debugging.disable_enemy_shooting) {
		handle_enemy_shooting(entity);
	}
}

// rotates enemy to face player
void AISystem::update_enemy_rotation(Entity entity, float elapsed_ms) {
	Entity player = registry.players.entities[0];
	auto& player_motion = registry.motions.get(player);
	auto& enemy_motion = registry.motions.get(entity);
	auto& enemy = registry.enemies.get(entity);

	// normalize angle for current to [0, 360)
	float currentAngle = fmod(enemy_motion.angle, 360.f);
	if (currentAngle < 0)

		currentAngle += 360.f;

	// normalize angle for desired to [0, 360)
	float desiredAngle = glm::degrees(atan2(
		player_motion.position.y - enemy_motion.position.y,
		player_motion.position.x - enemy_motion.position.x));
	desiredAngle = fmod(desiredAngle, 360.f);
	if (desiredAngle < 0)
		desiredAngle += 360.f;
	// get smallest angular diff in the range [-180, 180]
	float deltaAngle = fmod(desiredAngle - currentAngle + 540.f, 360.f) - 180.f;
	if (deltaAngle > 180.f) {
		deltaAngle -= 360.f;
	}
	if (deltaAngle < -180.f) {
		deltaAngle += 360.f;
	}

	// maximum rotation allowed this frame
	float maxTurn = enemy.turn_speed * (elapsed_ms / 1000.f);

	// rotate by either the remaining difference or by maxTurn
	float rotationStep = (fabs(deltaAngle) < maxTurn) ? fabs(deltaAngle) : maxTurn;
	if (deltaAngle < 0) {
		enemy_motion.angle = currentAngle - rotationStep;
	}
	else {
		enemy_motion.angle = currentAngle + rotationStep;
	}

	// result normalized again
	enemy_motion.angle = fmod(enemy_motion.angle, 360.f);
	if (enemy_motion.angle < 0) {
		enemy_motion.angle += 360.f;
	}
}

// lerp function
void AISystem::move_toward_player_lerp(Entity entity, float elapsed_ms) {
	Entity player = registry.players.entities[0];
	auto& player_motion = registry.motions.get(player);
	auto& enemy_motion = registry.motions.get(entity);
	vec2 direction = normalize(player_motion.position - enemy_motion.position);

	vec2 target_velocity = direction * registry.enemies.get(entity).speed;
	float smooth_constant = 10.f;
	float time_adjusted_constant = 1.0f - expf(-smooth_constant * (elapsed_ms / 1000.f));
	enemy_motion.velocity = lerp(enemy_motion.velocity, target_velocity, time_adjusted_constant);
}

// regular function no lerp
void AISystem::move_toward_player(Entity entity) {
	Entity player = registry.players.entities[0];
	auto& player_motion = registry.motions.get(player);
	auto& enemy_motion = registry.motions.get(entity);
	vec2 direction = normalize(player_motion.position - enemy_motion.position);

	enemy_motion.velocity = direction * registry.enemies.get(entity).speed;
}

// shoots projectile from enemy
void AISystem::handle_enemy_shooting(Entity entity) {
	auto& enemy_motion = registry.motions.get(entity);
	auto& gun = registry.guns.get(entity);

	if (gun.cooldown_timer_ms > 0.) {
		return;
	}

	if (registry.players.entities.empty()) {
		return; // No player to target
	}

	Entity player = registry.players.entities[0];
	auto& player_motion = registry.motions.get(player);

	// aim threshold calculation to only fire if facing player
	float desiredAngle = glm::degrees(atan2(player_motion.position.y - enemy_motion.position.y,
		player_motion.position.x - enemy_motion.position.x));
	desiredAngle = normalize_angle(desiredAngle);
	float currentAngle = normalize_angle(enemy_motion.angle);
	float angleDiff = fabs(desiredAngle - currentAngle);

	const float AIM_THRESHOLD = 5.f;  // only shoot if within 5 degress
	if (angleDiff > AIM_THRESHOLD) {
		return;
	}


	// enemy position calculations
	vec2 enemy_dcs_pos = this->renderer->wcs_to_dcs(enemy_motion.position);
	vec2 direction_to_player = normalize(player_motion.position - enemy_motion.position);
	vec2 projectile_velocity = direction_to_player * gun.projectile_speed;
	vec2 gun_position = get_enemy_gun_position(entity);

	// generate a random inaccuracy offset
	float angle_offset = ((rand() % 26) - 12.5);  // Random int between -12.5 and +12.5
	float inaccurate_angle = atan2(direction_to_player.y, direction_to_player.x) + glm::radians(angle_offset);

	for (int i = 0; i < gun.projectile_count; i++) {
		float spread_angle = 0.f;
		
		if (gun.projectile_count > 1) {
			float angle_step = gun.spread_cone / (gun.projectile_count - 1);
			spread_angle = -gun.spread_cone / 2.f + i * angle_step;
		}

		float inaccurate_spread = inaccurate_angle + glm::radians(spread_angle);
		projectile_velocity = vec2(cos(inaccurate_spread), sin(inaccurate_spread)) * gun.projectile_speed;
		
		createProjectile(
			gun_position, 
			{ 15, 15 }, 
			projectile_velocity, 
			glm::degrees(inaccurate_angle) + 180.0f + spread_angle,
			0.f, 
			gun.damage, 
			false, 
			TEXTURE_ASSET_ID::PROJECTILE
		);
	}
    add_temp_light(gun_position, {1.0, 0.6, 0.2}, 200, 1.f, true, 100.f);
	gun.cooldown_timer_ms = gun.cooldown_ms;
	create_animation(gun_position, enemy_motion.velocity, { 20.0f, 20.0f }, enemy_motion.angle, std::vector<TEXTURE_ASSET_ID>{TEXTURE_ASSET_ID::MUZZLE_FLASH0, TEXTURE_ASSET_ID::MUZZLE_FLASH1, TEXTURE_ASSET_ID::MUZZLE_FLASH2},
		true, false, 50.0f, Z_INDEX::MUZZLE_FLASH);
	audio->play_sound(gun.sound_effect, 10);

	// alert all enemies in the same room as the player
	for (Entity& other_enemy : registry.enemies.entities) {
		if (other_enemy == entity) {
			continue;
		}
		if (is_in_same_room(other_enemy, entity)) {
			if (registry.enemies.get(other_enemy).state == ENEMY_STATE::IDLE) {
				registry.enemies.get(other_enemy).state = ENEMY_STATE::PURSUIT;
			}
			// invalidate the cached path so a new path is computed
			if (registry.pathComponents.has(other_enemy)) {
				auto& pathComp = registry.pathComponents.get(other_enemy);
				pathComp.valid = false;
			}
		}
	}
}

vec2 AISystem::get_enemy_gun_position(Entity entity) {
	auto& enemy_motion = registry.motions.get(entity);
	vec2 gun_offset = vec2(enemy_motion.scale.x / 2, 0);
	float rad = glm::radians(enemy_motion.angle);
	vec2 rotated_offset = {
		gun_offset.x * cos(rad) - gun_offset.y * sin(rad),
		gun_offset.x * sin(rad) + gun_offset.y * cos(rad)
	};
	return enemy_motion.position + rotated_offset;
}

// todo: refactor this to instead iterate through the entities known already
// and then update the timer, like in AnimationSystem::progress_timers()
// this will be faster
void AISystem::progress_timers(float elapsed_ms, Entity entity) {
	auto& gun = registry.guns.get(entity);
	if (gun.cooldown_timer_ms > 0) {
		gun.cooldown_timer_ms = gun.cooldown_timer_ms - elapsed_ms;
	}
}

void AISystem::move_away_from_player(Entity enemy) {
	Entity player = registry.players.entities[0];
	auto& player_motion = registry.motions.get(player);
	auto& enemy_motion = registry.motions.get(enemy);
	vec2 direction = normalize(player_motion.position - enemy_motion.position);
	// todo: insert stuff here to check collisions

	enemy_motion.velocity = -direction * registry.enemies.get(enemy).speed;
}

bool AISystem::is_in_same_room(Entity a, Entity b) {
	// Make sure both entities have Motion components. 
	auto& aMotion = registry.motions.get(a);
	auto& bMotion = registry.motions.get(b);
	int current_level = registry.gameProgress.components[0].level;
	Map& map = registry.maps.components[current_level];
	ivec2 aGrid = world_to_grid_coords(aMotion.position.x, aMotion.position.y);
	ivec2 bGrid = world_to_grid_coords(bMotion.position.x, bMotion.position.y);
	return (map.room_mask[aGrid.y][aGrid.x] == map.room_mask[bGrid.y][bGrid.x]);
}

// Normalize an angle to the range [-180, 180] degrees.
float AISystem::normalize_angle(float angle) {
	while (angle > 180.f) {
		angle -= 360.f;
	}
	while (angle < -180.f) {
		angle += 360.f;
	}
	return angle;
}