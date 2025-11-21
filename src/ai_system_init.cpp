#include "ai_system_init.hpp"
#include "animation_init.hpp"
#include "common.hpp"
#include "world_init.hpp"
#include "map_system.hpp"
#include "guns.hpp"
#include <iostream>

Entity create_enemy(ivec2 grid_position, GUN_TYPE gun_type, float health, float speed_factor, float detection_range_factor, float attack_range_factor) {
	auto entity = Entity();
	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.health = health;
	enemy.speed = GRID_CELL_SIZE * speed_factor;
	enemy.pursuit_range = GRID_CELL_SIZE * detection_range_factor;
	enemy.attack_range = GRID_CELL_SIZE * attack_range_factor;

	if (gun_type == GUN_TYPE::DUMMY_GUN) {
		registry.tutorialEnemies.emplace(entity);
	}

	// Give the enemy a gun
	create_gun(entity, gun_type);

	// motion component
	auto& enemyMotion = registry.motions.emplace(entity);
	enemyMotion.position = grid_to_world_coord(grid_position.x, grid_position.y);
	enemyMotion.angle = static_cast<float>(get_rand(0, 360));
	enemyMotion.velocity = glm::vec2(0.0f, 0.0f);
	enemyMotion.scale = glm::vec2(GRID_CELL_SIZE * 1.7, GRID_CELL_SIZE * 1.7);

	registry.movingCollidables.emplace(entity);
	registry.movingCircleCollidables.emplace(entity);
	Collidable& collidable = registry.collidables.emplace(entity);
	CircleBound& circle_bound = registry.circlebounds.emplace(entity);
	circle_bound.collision_radius = GRID_CELL_SIZE / 2.;
	circle_bound.offset = { 0.f, 0.f };

	// initial state
	enemy.state = ENEMY_STATE::IDLE;

	// rendering using player texture right now todo: change to a different texture
	registry.renderRequests.emplace(entity, RenderRequest{
		TEXTURE_ASSET_ID::ENEMY,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::PLAYER
		});
	return entity;
}

void create_gun(Entity entity, GUN_TYPE gun_type) {
	if (gun_type == GUN_TYPE::PISTOL) {
		registry.guns.emplace(entity, PISTOL);
	} 
	else if (gun_type == GUN_TYPE::ENEMY_PISTOL) {
		registry.guns.emplace(entity, ENEMY_PISTOL);
	} else if (gun_type == GUN_TYPE::DUMMY_GUN) {
		registry.guns.emplace(entity, DUMMY_GUN);
	}
	else if (gun_type == GUN_TYPE::SHOTGUN) {
		registry.guns.emplace(entity, SHOTGUN);
	}
	else if (gun_type == GUN_TYPE::SMG) {
		registry.guns.emplace(entity, SMG);
	}
	else if (gun_type == GUN_TYPE::RAILGUN) {
		registry.guns.emplace(entity, RAILGUN);
	}
	else if (gun_type == GUN_TYPE::REVOLVER) {
		registry.guns.emplace(entity, REVOLVER);
	}
}

void enemy_got_shot(Entity enemy, Entity projectile, AudioSystem* audio) {
	audio->play_sound(SOUND_ASSET_ID::BULLET_HIT_FLESH_1, 30);

	auto& enemy_comp = registry.enemies.get(enemy);
	auto& enemy_motion = registry.motions.get(enemy);
	auto& projectile_motion = registry.motions.get(projectile);
	auto& projectile_comp = registry.projectiles.get(projectile);

	enemy_comp.health -= projectile_comp.damage;

	// Determine the room of the enemy that got shot.
	int current_level = registry.gameProgress.components[0].level;
	Map& map = registry.maps.components[current_level];
	ivec2 enemyGrid = world_to_grid_coords(enemy_motion.position.x, enemy_motion.position.y);
	int room_id = map.room_mask[enemyGrid.y][enemyGrid.x];
	// Alert all enemies in that room.
	alert_enemies_in_room(room_id);

	if (projectile_comp.is_gun) {
		vec2 spawn_offset = normalize(projectile_motion.velocity) * 75.0f;

		// Spawn the gun slightly offset from the enemy's position
		Gun gun = registry.guns.get(projectile);
		Entity hit_entity = spawn_pickup(enemy_motion.position - spawn_offset, 0, PICKUP_TYPE::GUN, gun.current_magazine + gun.remaining_bullets, gun.gun_type);
		registry.guns.remove(projectile);
	}


	// alert the enemy to pursue the player upon being hit
	enemy_comp.state = ENEMY_STATE::PURSUIT;
	// invalidate  path so that the enemy recalculates a route to the player
	if (registry.pathComponents.has(enemy)) {
		registry.pathComponents.get(enemy).valid = false;
	}

	if (enemy_comp.health <= 0) {
		SOUND_ASSET_ID random_enemy_hit_sound = static_cast<SOUND_ASSET_ID>(get_rand(static_cast<int>(SOUND_ASSET_ID::ENEMY_HIT_1), static_cast<int>(SOUND_ASSET_ID::ENEMY_HIT_8)));
		audio->play_sound(random_enemy_hit_sound, 5);
		create_dead_enemy(enemy_motion.position, enemy_motion.angle);
		Gun& enemy_gun = registry.guns.get(enemy);
		GUN_TYPE pickup_gun_type = GUN_TYPE::PISTOL;
		if (enemy_gun.gun_type != GUN_TYPE::ENEMY_PISTOL && enemy_gun.gun_type != GUN_TYPE::DUMMY_GUN) {
			// If we create new specialized guns (e.g. ENEMY_SHOTGUN, ENEMY_SMG) for enemies, we have to change the logic here so that enemy guns can be mapped to player guns.
			pickup_gun_type = enemy_gun.gun_type;
		}
		spawn_pickup(enemy_motion.position, 0, PICKUP_TYPE::GUN, 10, pickup_gun_type);
		registry.guns.remove(enemy);

		registry.remove_all_components_of(enemy);

		if (registry.enemies.size() - registry.tutorialEnemies.size() <= 0) {
			// TODO: Why do we do this check in two places
			std::cout << "Level cleared" << std::endl;
			registry.map_system->load_next_map();
		}
	}
	else {
		enemy_motion.velocity += normalize(enemy_motion.position - projectile_motion.position) * 100.f;
	}

	create_animation(
		enemy_motion.position, 
		enemy_motion.velocity,
		{ 150.f, 150.f },
		projectile_motion.angle + 90.f,
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

// Alerts all enemies in the given room.
void alert_enemies_in_room(int room_id) {
	int current_level = registry.gameProgress.components[0].level;
	Map& map = registry.maps.components[current_level];
	for (Entity enemy : registry.enemies.entities) {
		auto& enemy_motion = registry.motions.get(enemy);
		ivec2 enemyGrid = world_to_grid_coords(enemy_motion.position.x, enemy_motion.position.y);
		if (map.room_mask[enemyGrid.y][enemyGrid.x] == room_id) {
			registry.enemies.get(enemy).state = ENEMY_STATE::PURSUIT;
			if (registry.pathComponents.has(enemy)) {
				registry.pathComponents.get(enemy).valid = false;
			}
		}
	}
}

void create_dead_enemy(vec2 pos, float angle) {
	auto entity = Entity();
	registry.deadEnemies.emplace(entity);

	auto& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = angle;
	motion.velocity = glm::vec2(0.0f, 0.0f);
	motion.scale = glm::vec2(GRID_CELL_SIZE * 2.1f, GRID_CELL_SIZE * 2.1f);

	registry.renderRequests.emplace(entity, RenderRequest{
		TEXTURE_ASSET_ID::DEAD_ENEMY,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::DEAD_ENEMY
	});
}