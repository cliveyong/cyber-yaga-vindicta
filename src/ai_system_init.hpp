#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "audio_system.hpp"

Entity create_enemy(ivec2 grid_position, GUN_TYPE gun_type, float health, float speed_factor, float detection_range_factor, float attack_range_factor);

void create_gun(Entity entity, GUN_TYPE gun_type);

void enemy_got_shot(Entity enemy, Entity projectile, AudioSystem* audio);

void create_dead_enemy(vec2 pos, float angle);

void alert_enemies_in_room(int room_id);