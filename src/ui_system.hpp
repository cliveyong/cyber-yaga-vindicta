#pragma once
#include "common.hpp"
#include "tinyECS/entity.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include <render_system.hpp>
#include <audio_system.hpp>

class WorldSystem;

class UISystem {
public:
    void init(GLFWwindow* window, WorldSystem* world_system, RenderSystem* render_system, AudioSystem* audio_system);

    void update_stamina_bar();

    void update_health_bar();

    void update_ammo_counter();

    Entity create_stamina_bar();

    Entity create_health_bar();

    Entity create_gun_ui();

    void update_gun_ui();

    Entity create_ammo_ui();

    void update_UI();

    void step(float elapsed_ms);

    void progress_timers(float elapsed_ms);

    void update_render(Entity& entity);

    void createUI();

    void display_title_screen();

    void display_title_screen_text();

    void display_given_text(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_ID);

    void add_buttons();

    void check_if_button_pressed();

    void hide_title_screen();

    Entity display_given_button(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_ID, ButtonType type);


private:
    int health_frames, stamina_frames;
    Entity staminabar_entity;
    Entity healthbar_entity;

    Entity title_screen_entity;

    float cinematic_timer;

    GLFWwindow* window;
    WorldSystem* world_system;
    RenderSystem* render_system;
    AudioSystem* audio_system;
    Entity gun_entity;
    Entity ammo_entity, max_ammo_entity;
    Entity cinematic_entity;
};


