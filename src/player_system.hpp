#pragma once
#include "common.hpp"
#include "tinyECS/entity.hpp"
#include "render_system.hpp"
#include "audio_system.hpp"
#include <world_system.hpp>

class PlayerSystem {
public:
    // Updates the player's rotation to face the mouse
    void updatePlayerRotation();

	bool step(float elapsed_ms);

	void init(RenderSystem* renderer, AudioSystem* audio, WorldSystem* world, GLFWwindow* window);

    void progress_timers(float elapsed_ms);

    Entity createPlayer(vec2 position);

    Entity create_laser();

    void create_crosshair();

    void handle_melee();

    void handleShooting();

    void handle_dash(float elapsed_ms);

    void update_player_velocity(float elapsed_ms);

    void update_camera_position(float elapsed_ms);

    void update_crosshair_position();

    void handle_reload();

    void handle_exit();


private:
    Entity player;
    Entity laser;
    Entity crosshair;

	RenderSystem* renderer;
    AudioSystem* audio;
    WorldSystem* world;
	GLFWwindow* window;

    vec2 get_gun_position();

    vec2 get_laser_position();
};

void player_got_shot(Entity projectile, AudioSystem* audio);

void update_player_sprite();
