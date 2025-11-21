#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "audio_system.hpp"

#include <memory>
#include "decision_tree_ai.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);

	void init(RenderSystem* renderer, AudioSystem* audio);

	Entity create_enemy(vec2 pos);

	Entity create_mesh_enemy(vec2 pos, std::vector<vec2> vertices);

	Entity create_enemy_with_gun(vec2 pos);

	Entity create_mesh_enemy_with_gun(vec2 pos, std::vector<vec2> vertices);

	void create_gun(Entity entity, float speed, float cooldown);

	void update_player_detection(Entity entity);

	float get_distance_to_player(Entity enemy);

	void shoot_if_aggro(Entity entity, float elapsed_ms);

	void update_enemy_rotation(Entity entity, float elapsed_ms);

	void move_toward_player_lerp(Entity entity, float elapsed_ms);

	void move_toward_player(Entity entity);

	void handle_enemy_shooting(Entity entity);

	vec2 get_enemy_gun_position(Entity entity);

	void progress_timers(float elapsed_ms, Entity entity);

	void update_velocity(Entity entity, float elapsed_ms);

	void move_away_from_player(Entity enemy);

	bool is_in_same_room(Entity a, Entity b);

	float normalize_angle(float angle);

private:

	RenderSystem* renderer;

	AudioSystem* audio;

	std::unique_ptr<decision_node> decisionTree;

};

