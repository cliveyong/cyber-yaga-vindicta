#include "ui_system.hpp"
#include "common.hpp"
#include "world_system.hpp"
#include "render_system.hpp"
#include "audio_system.hpp"
#include "animation_init.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include <iostream>


// initialize animation system
void UISystem::init(GLFWwindow* window, WorldSystem *world_system, RenderSystem* render_system, AudioSystem* audio) {
	this->window = window;
	this->world_system = world_system;
	this->render_system = render_system;
	this->audio_system = audio;
	health_frames = 0;
	Entity cinematic_timer = Entity();
	createUI();
	display_title_screen();
	display_title_screen_text();
	add_buttons();
	return;
}

// ================================
// Update UI Functions
// ================================
// updates UI
void UISystem::step(float elapsed_ms) {
	progress_timers(elapsed_ms);

	auto& inputs = registry.inputs.get(registry.inputs.entities[0]);
	if (world_system->get_game_state() == GameState::TITLE_SCREEN) {
		if (inputs.key_down_events[GLFW_MOUSE_BUTTON_LEFT]) {
			check_if_button_pressed();	
		}
		return;
	} else if (world_system->get_game_state() == GameState::CINEMATIC) {
		// If user clicks, skip the cinematic
		if (inputs.keys[GLFW_KEY_SPACE] || inputs.keys[GLFW_KEY_ENTER]) {
			cinematic_timer = 0.0f;
			registry.remove_all_components_of(cinematic_entity);
		}
		if (cinematic_timer <= 0.0f) {
			if (!debugging.disable_music) {
				this->audio_system->play_music(MUSIC_ASSET_ID::MUSIC1, 14);
			}
			// THE LOBBY animation
			create_animation(
				{registry.screen_state.resolution_x/2, registry.screen_state.resolution_y/2},
				{ 0.f, 0.f },
				{registry.screen_state.resolution_x, registry.screen_state.resolution_y},
				0.f,
				std::vector<TEXTURE_ASSET_ID>{
					TEXTURE_ASSET_ID::LEVEL_0_TEXT
				},
				true,
				false,
				3000.0f,
				Z_INDEX::CUTSCENE,
				true,
				true
			);
			world_system->set_game_state(GameState::PLAYING);
		}
		return;
	}
	update_UI();
}

void UISystem::progress_timers(float elapsed_ms)
{
	cinematic_timer -= elapsed_ms;
}

// updates all UI components
void UISystem::update_UI()
{
	update_health_bar();
	update_stamina_bar();
	update_ammo_counter();
}

// updates stamina bar
void UISystem::update_stamina_bar() {
	Entity& entity = registry.players.entities[0];
	Dash& dash = registry.dashes.get(entity);
	float stamina_percent = (dash.dash_cooldown_ms - dash.cooldown_timer) / dash.dash_cooldown_ms;
	int new_frame = stamina_percent * stamina_frames - 1;
	if (new_frame <= 0) {
		new_frame = 0;
	}
	UI& ui = registry.uis.get(staminabar_entity);
	ui.state = new_frame;
	update_render(staminabar_entity);
}

// update health bar
void UISystem::update_health_bar() {
	Entity& entity = registry.players.entities[0];
	Player& player = registry.players.get(entity);
	float health_percent = player.health / STARTING_PLAYER_HEALTH;
	int new_frame = health_percent * health_frames - 1;

	UI& ui = registry.uis.get(healthbar_entity);
	ui.state = new_frame;
	update_render(healthbar_entity);
}

// update ammo counter
void UISystem::update_ammo_counter() {
	Entity& player = registry.players.entities[0];
	if (!registry.guns.has(player)) {
		Text& ammo_counter = registry.texts.get(ammo_entity);
		ammo_counter.color = {1.f,0.25f,0.25f};
		ammo_counter.content = "x 0";

		Text& max_ammo_counter = registry.texts.get(max_ammo_entity);
		max_ammo_counter.content = "0";
		return;
	}
	auto& gun = registry.guns.get(player);
	Text& ammo_counter = registry.texts.get(ammo_entity);
	ammo_counter.content = "x " + std::to_string((int)gun.current_magazine);
	if (gun.current_magazine == 0) {
		ammo_counter.color = {1.f,0.25f,0.25f};
	} else {
		ammo_counter.color = {1.f,1.f,1.f};
	}

	Text& max_ammo_counter = registry.texts.get(max_ammo_entity);
	max_ammo_counter.content = std::to_string((int)gun.remaining_bullets);
}

// updates render of entity
void UISystem::update_render(Entity& entity) {
RenderRequest& rr = registry.renderRequests.get(entity);
UI& ui_component = registry.uis.get(entity);
rr.used_texture = ui_component.textures[ui_component.state];
}

// ================================
// Create UI Functions
// ================================

// initizlizes stamina bar
Entity UISystem::create_stamina_bar() {
	Entity stamina_entity = Entity();
	UI& stamina_ui = registry.uis.emplace(stamina_entity);
	stamina_ui.textures = std::vector<TEXTURE_ASSET_ID>{ TEXTURE_ASSET_ID::STAMINA_0, TEXTURE_ASSET_ID::STAMINA_1,
	TEXTURE_ASSET_ID::STAMINA_2, TEXTURE_ASSET_ID::STAMINA_3, TEXTURE_ASSET_ID::STAMINA_4, TEXTURE_ASSET_ID::STAMINA_5,
	TEXTURE_ASSET_ID::STAMINA_6, TEXTURE_ASSET_ID::STAMINA_7, TEXTURE_ASSET_ID::STAMINA_8, TEXTURE_ASSET_ID::STAMINA_9,
	TEXTURE_ASSET_ID::STAMINA_10, TEXTURE_ASSET_ID::STAMINA_11, TEXTURE_ASSET_ID::STAMINA_12, TEXTURE_ASSET_ID::STAMINA_13,
	TEXTURE_ASSET_ID::STAMINA_14, TEXTURE_ASSET_ID::STAMINA_15, TEXTURE_ASSET_ID::STAMINA_16 };

	stamina_frames = stamina_ui.textures.size();

	Motion& s_motion = registry.motions.emplace(stamina_entity);
	s_motion.position = { 130, WINDOW_HEIGHT_PX - 40 };
	s_motion.scale = glm::vec2(260.0f, 80.0f);

	registry.renderRequests.emplace(stamina_entity, RenderRequest{
		stamina_ui.textures[8],
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::HUD
		});

	return stamina_entity;
}

// initializes health bar
Entity UISystem::create_health_bar() {
	Entity health_border_entity = Entity();
	UI& health_border_ui = registry.uis.emplace(health_border_entity);
	health_border_ui.textures = std::vector<TEXTURE_ASSET_ID>{ TEXTURE_ASSET_ID::HEALTH_BORDER };

	Motion& motion = registry.motions.emplace(health_border_entity);
	motion.position = { WINDOW_WIDTH_PX / 13, WINDOW_HEIGHT_PX - 80 };
	motion.scale = glm::vec2(250.0f, 80.0f);

	Entity health_entity = Entity();
	UI& health_ui = registry.uis.emplace(health_entity);
	health_ui.textures = std::vector<TEXTURE_ASSET_ID>{ TEXTURE_ASSET_ID::HEALTH_1, TEXTURE_ASSET_ID::HEALTH_2,
	TEXTURE_ASSET_ID::HEALTH_3, TEXTURE_ASSET_ID::HEALTH_4, TEXTURE_ASSET_ID::HEALTH_5, TEXTURE_ASSET_ID::HEALTH_6 };
	health_frames = health_ui.textures.size();

	Motion& h_motion = registry.motions.emplace(health_entity);
	h_motion.position = { WINDOW_WIDTH_PX / 13, WINDOW_HEIGHT_PX - 80 };
	h_motion.scale = glm::vec2(250.0f, 80.0f);

	registry.renderRequests.emplace(health_border_entity, RenderRequest{
		TEXTURE_ASSET_ID::HEALTH_BORDER,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::HUD
	});

	registry.renderRequests.emplace(health_entity, RenderRequest{
		health_ui.textures[0],
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::HUD
	});

	return health_entity;
}

Entity UISystem::create_gun_ui()
{
	Entity gun_entity = Entity();
	UI& gun_ui = registry.uis.emplace(gun_entity);
	gun_ui.textures = std::vector<TEXTURE_ASSET_ID>{ TEXTURE_ASSET_ID::DEAD_ENEMY };

	Motion& gun_motion = registry.motions.emplace(gun_entity);
	gun_motion.position = { 85, WINDOW_HEIGHT_PX - 150 };
	gun_motion.scale = glm::vec2(100.0f, 100.0f);

	registry.renderRequests.emplace(gun_entity, RenderRequest{
		TEXTURE_ASSET_ID::PISTOL,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		Z_INDEX::HUD
		});

	return gun_entity;
}

void UISystem::update_gun_ui() {
	Entity player_entity = registry.players.entities[0];
	RenderRequest& render = registry.renderRequests.get(gun_entity);
	if (!registry.guns.has(player_entity)) {
		render.alpha = 0.f;
		return;
	}

	render.alpha = 1.f;
	Gun& gun = registry.guns.get(player_entity);
	render.used_texture = gun.hud_sprite;
}

Entity UISystem::create_ammo_ui()
{
	Entity ammo_entity = Entity();
	UI& ammo_ui = registry.uis.emplace(ammo_entity);

	Motion& ammo_motion = registry.motions.emplace(ammo_entity);
	ammo_motion.position = { 150, WINDOW_HEIGHT_PX - 140 };
	ammo_motion.scale = glm::vec2(1.0f, 1.0f);

	Entity& player = registry.players.entities[0];
	auto& gun = registry.guns.get(player);
	Text& ammo_counter = registry.texts.emplace(ammo_entity);
	ammo_counter.color = { 1.f, 1.f, 1.f };
	ammo_counter.content = "x " + std::to_string((int)gun.current_magazine);

	Entity max_ammo_entity = Entity();
	UI& max_ammo_ui = registry.uis.emplace(max_ammo_entity);

	Motion& max_ammo_motion = registry.motions.emplace(max_ammo_entity);
	max_ammo_motion.position = { 260, WINDOW_HEIGHT_PX - 140 };
	max_ammo_motion.scale = glm::vec2(.70f, .70f);
	this->max_ammo_entity = max_ammo_entity;

	Text& max_ammo_counter = registry.texts.emplace(max_ammo_entity);
	max_ammo_counter.color = { 0.5f, 0.5f, 0.5f };
	max_ammo_counter.content = std::to_string((int)gun.remaining_bullets);

	return ammo_entity;
}


// creates UI
void UISystem::createUI()
{
	staminabar_entity = create_stamina_bar();
	healthbar_entity = create_health_bar();
	gun_entity = create_gun_ui();
	ammo_entity = create_ammo_ui();
}

void UISystem::display_title_screen()
{
	title_screen_entity = Entity();
	auto& motion = registry.motions.emplace(title_screen_entity);

	motion.position = { WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 };
	motion.scale = {WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX};
	
	registry.uis.emplace(title_screen_entity);
	registry.renderRequests.emplace(title_screen_entity, RenderRequest{
			TEXTURE_ASSET_ID::TITLE_SCREEN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::TITLE_SCREEN,
			true,
		});


}
 
void UISystem::display_title_screen_text()
{
	display_given_text({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 80}, { 250, 80 }, TEXTURE_ASSET_ID::START_TEXT);

	// display_given_text({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 80 }, { 250, 80 }, TEXTURE_ASSET_ID::RESUME_TEXT);

	// display_given_text({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 160 }, { 250, 80 }, TEXTURE_ASSET_ID::OPTIONS_TEXT);

	display_given_text({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 160 }, { 250, 80 }, TEXTURE_ASSET_ID::QUIT_TEXT);

}

void UISystem::display_given_text(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_ID)
{
	auto text_entity = Entity();
	auto& text = registry.text.emplace(text_entity);

	text.position = position;
	text.scale = scale;

	registry.uis.emplace(text_entity);
	registry.renderRequests.emplace(text_entity, RenderRequest{
			texture_ID,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::TITLE_SCREEN_BUTTON,
			true,
		});
}


void UISystem::add_buttons()
{
	// Start
	display_given_button({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 80 }, { 250, 80 }, TEXTURE_ASSET_ID::BUTTON_TESTER, ButtonType::START);

	// Resume
	// display_given_button({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 80 }, { 250, 80 }, TEXTURE_ASSET_ID::BUTTON_TESTER, ButtonType::RESUME);

	// Options 
	// display_given_button({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 160 }, { 250, 80 }, TEXTURE_ASSET_ID::BUTTON_TESTER, ButtonType::OPTIONS);

	// Quit
	display_given_button({ (WINDOW_WIDTH_PX / 2) + 630, (WINDOW_HEIGHT_PX / 2) + 160 }, { 250, 80 }, TEXTURE_ASSET_ID::BUTTON_TESTER, ButtonType::QUIT);
	
}


Entity UISystem::display_given_button(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_ID, ButtonType type)
{
	auto button_entity = Entity();
	UIButton& button = registry.buttons.emplace(button_entity);
	button.position = position;
	button.scale = scale;
	button.type = type;

	if (debugging.enable_button_outlines) {
		registry.uis.emplace(button_entity);
		registry.renderRequests.emplace(button_entity, RenderRequest{
				texture_ID,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE,
				Z_INDEX::TITLE_SCREEN_BUTTON,
				true,
			});
	}

	return button_entity;
}


void UISystem::check_if_button_pressed()
{
	auto& inputs = registry.inputs.get(registry.inputs.entities[0]);
	float mouse_pos_x = inputs.mouse_pos.x;
	float mouse_pos_y = inputs.mouse_pos.y;

	for (Entity button : registry.buttons.entities) {
		UIButton& btn = registry.buttons.get(button);

		vec2 start_pos = btn.position - btn.scale / 2.0f;
		vec2 end_pos = btn.position + btn.scale / 2.0f;

		if (mouse_pos_x >= start_pos.x && mouse_pos_x <= end_pos.x  &&
			mouse_pos_y >= start_pos.y &&  mouse_pos_y <= end_pos.y ) {

			if (btn.type == ButtonType::START) {
				world_system->set_game_state(GameState::CINEMATIC);
				cinematic_timer = 5000.0f * 9;
				cinematic_entity = create_animation(
					{ registry.screen_state.resolution_x / 2, registry.screen_state.resolution_y / 2 },
					{ 0.f, 0.f },
					{ registry.screen_state.resolution_x, registry.screen_state.resolution_y },
					0.f,
					std::vector<TEXTURE_ASSET_ID>{
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_0,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_1,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_2,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_3,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_4,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_5,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_6,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_7,
						TEXTURE_ASSET_ID::CINEMAITC_CUTSCENE_8,
					},
					true,
					false,
					5000.0f,
					Z_INDEX::CUTSCENE,
					true,
					true
				);
				hide_title_screen();
				update_gun_ui();
			}
			else if (btn.type == ButtonType::QUIT) {
				this->world_system->close_window();
			}

		}
	}
}

void UISystem::hide_title_screen()
{
	registry.remove_all_components_of(title_screen_entity);

	for (Entity e : registry.text.entities) {
		registry.remove_all_components_of(e);
	}

	for (Entity e : registry.buttons.entities) {
		registry.remove_all_components_of(e);
	}
}

