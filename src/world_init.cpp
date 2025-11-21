#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include "map_init.hpp"
#include "ai_system_init.hpp"
#include "player_system.hpp"
#include <iostream>

Entity createProjectile(vec2 pos, vec2 size, vec2 velocity, float angle, float angle_velocity, float damage, bool shot_by_player, TEXTURE_ASSET_ID texture_id, bool is_gun, bool can_bounce, int penetration_count, int ricochet_count)
{
	auto entity = Entity();
	auto& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.velocity = velocity;
	motion.scale = size;
	motion.angle = angle;
	motion.angle_velocity = angle_velocity;

	Projectile& projectile = registry.projectiles.emplace(entity);
	projectile.shot_by_player = shot_by_player;
	projectile.damage = damage;
	projectile.is_gun = is_gun;
	projectile.can_bounce = can_bounce;
	projectile.remaining_penetrations = penetration_count;
	projectile.ricochet_remaining = ricochet_count;

	registry.movingCollidables.emplace(entity);
	registry.movingSATCollidables.emplace(entity);
	registry.collidables.emplace(entity);
	AABB& aabb = registry.AABBs.emplace(entity);
	aabb.collision_box = motion.scale;
	aabb.offset = { 0.f, 0.f };

	registry.renderRequests.emplace(entity, RenderRequest{
		texture_id,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::PROJECTILE
		});

	return entity;
}

Entity spawn_pickup(vec2 position, float angle, PICKUP_TYPE pickup_type, float value, GUN_TYPE gun_type) {

	Entity entity = Entity();
	Pickup& pickup = registry.pickups.emplace(entity);
	pickup.gun_type = gun_type;
	pickup.type = pickup_type;
	pickup.value = value;
	pickup.range = GRID_CELL_SIZE*1.5;

	if (pickup_type == PICKUP_TYPE::GUN && gun_type == GUN_TYPE::GUN_COUNT) {
		std::cout << "Invalid Pickup" << std::endl;
		return entity;
	}

	auto& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = (pickup_type == PICKUP_TYPE::GUN) ? static_cast<float>(get_rand(0, 360)) : angle;
	motion.velocity = glm::vec2(0.0f, 0.0f);
	motion.scale = glm::vec2(GRID_CELL_SIZE * 0.75, GRID_CELL_SIZE * 0.75);

	TEXTURE_ASSET_ID pickup_texture = TEXTURE_ASSET_ID::HEALTH_BOX;
	
	if (pickup_type == PICKUP_TYPE::GUN) {
		// TODO: Refactor this into a map when we have more weapons
		if (gun_type == GUN_TYPE::PISTOL || gun_type == GUN_TYPE::ENEMY_PISTOL || gun_type == GUN_TYPE::DUMMY_GUN) {
			pickup_texture = TEXTURE_ASSET_ID::PISTOL_PICKUP;
			motion.scale = glm::vec2(GRID_CELL_SIZE * 0.75, GRID_CELL_SIZE * 0.75);
		} else if (gun_type == GUN_TYPE::SHOTGUN) {
			pickup_texture = TEXTURE_ASSET_ID::SHOTGUN_PICKUP;
			motion.scale = vec2(GRID_CELL_SIZE*1.5, GRID_CELL_SIZE*1.5);
		} else if (gun_type == GUN_TYPE::SMG) {
			pickup_texture = TEXTURE_ASSET_ID::SMG_PICKUP;
			motion.scale = vec2(GRID_CELL_SIZE, GRID_CELL_SIZE);
		} else if (gun_type == GUN_TYPE::RAILGUN) {
			pickup_texture = TEXTURE_ASSET_ID::RAILGUN_PICKUP;
			motion.scale = vec2(GRID_CELL_SIZE*1.5, GRID_CELL_SIZE*1.5);
		} else if (gun_type == GUN_TYPE::REVOLVER) {
			pickup_texture = TEXTURE_ASSET_ID::REVOLVER_PICKUP;
			motion.scale = glm::vec2(GRID_CELL_SIZE * 0.75, GRID_CELL_SIZE * 0.75);
		}
	}

	registry.renderRequests.emplace(entity, RenderRequest{
		pickup_texture,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::PICKUP
	});

	registry.textureWithoutLighting.emplace(entity);

	return entity;
}

void collect_pickup(Entity pickup_entity, AudioSystem* audio) {
	if (!registry.pickups.has(pickup_entity)) {
		return;
	}

	Pickup& pickup = registry.pickups.get(pickup_entity);
	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.components[0];

	const PICKUP_TYPE pickup_type = pickup.type;
	SOUND_ASSET_ID pickup_sound;
	bool pickup_gun = false;
	switch (pickup_type) {
	case PICKUP_TYPE::HEALTH_BOX:
		if (player.health == STARTING_PLAYER_HEALTH) {
			return; // Don't use the health box
		}

		player.health = min(STARTING_PLAYER_HEALTH, player.health + pickup.value);
		audio->play_sound(SOUND_ASSET_ID::HEALTH_BOOST, 20);
		break;

	case PICKUP_TYPE::GUN:
		if (registry.guns.has(registry.players.entities[0])) {
			Gun& player_gun = registry.guns.get(registry.players.entities[0]);

			if (player_gun.gun_type == pickup.gun_type) {
				player_gun.remaining_bullets += pickup.value;
			} else {
				// If the pickup gun_type is different from the player's gun, don't pick it up
				// Only pick it up when the player has thrown away its gun and has no gun.
				// TODO: display "RIGHT CLICK TO SWAP"
				return;
			}
		} else {
			create_gun(player_entity, pickup.gun_type);
			Gun& player_gun = registry.guns.get(registry.players.entities[0]);
			player_gun.current_magazine = std::min(player_gun.magazine_size, pickup.value);
			player_gun.remaining_bullets = std::max(0, pickup.value - player_gun.current_magazine);
			registry.ui_system->update_gun_ui();
			update_player_sprite();
		}
		break;
	}
	pickup_sound = static_cast<SOUND_ASSET_ID>(get_rand(static_cast<int>(SOUND_ASSET_ID::PISTOL_LOAD_1), static_cast<int>(SOUND_ASSET_ID::PISTOL_LOAD_5)));
	audio->play_sound(pickup_sound, 20);
	registry.remove_all_components_of(pickup_entity);
}

vec2 grid_to_world_coord(float x, float y) {
	return vec2(x, y) * GRID_CELL_SIZE + vec2(GRID_CELL_SIZE / 2, GRID_CELL_SIZE / 2);
}

ivec2 world_to_grid_coords(float x, float y) {
	return {
		(int)(x / GRID_CELL_SIZE),
		(int)(y / GRID_CELL_SIZE)
	};
}

void create_debris(vec2 door_loc, vec2 away_from_player, int rng)
{
	// Create door debris
	std::vector<TEXTURE_ASSET_ID> textures = {
		TEXTURE_ASSET_ID::DOOR_PARTICLE_0, TEXTURE_ASSET_ID::DOOR_PARTICLE_1, TEXTURE_ASSET_ID::DOOR_PARTICLE_2,
		TEXTURE_ASSET_ID::DOOR_PARTICLE_3, TEXTURE_ASSET_ID::DOOR_PARTICLE_4, TEXTURE_ASSET_ID::DOOR_PARTICLE_5,
		TEXTURE_ASSET_ID::DOOR_PARTICLE_6, TEXTURE_ASSET_ID::DOOR_PARTICLE_7, TEXTURE_ASSET_ID::DOOR_PARTICLE_8,
		TEXTURE_ASSET_ID::DOOR_PARTICLE_9
	};

	std::vector<vec2> directions = {
		vec2(1.f, 0.f),  // right
		vec2(-1.f, 0.f), // left
		vec2(0.f, 1.f),  // up
		vec2(0.f, -1.f)  // down
	};

	int num_debris = std::max(rng, 6);
	for (int i = 0; i < num_debris; i++) {
		Entity debris = Entity();

		// Slight offset near the door center for placement
		float offset_x = (rand() % 10 - 5) * 0.5f;
		float offset_y = (rand() % 10 - 5) * 0.5f;
		vec2 debris_position = door_loc + vec2(offset_x, offset_y);

		// Pick direction cyclically
		vec2 base_dir = directions[i % directions.size()];

		// Add slight randomness to direction for spread
		vec2 random_offset = vec2((rand() % 100 - 50) / 500.f, (rand() % 100 - 50) / 500.f);
		vec2 debris_velocity = normalize(base_dir + random_offset) * (600.f + (rand() % 50));

		vec2 debris_scale = vec2(100.f, 100.f); // Size of debris

		createProjectile(debris_position, debris_scale, debris_velocity, 0, 0, PROJECTILE_DAMAGE * 3, true, textures[i % textures.size()], false, true);

		registry.renderRequests.insert(
			debris,
			{
				textures[i % textures.size()],
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE,
				Z_INDEX::PROP
			}
		);
	}
}
