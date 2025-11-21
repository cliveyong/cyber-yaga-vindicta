
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "input_system.hpp"
#include "player_system.hpp"
#include "audio_system.hpp"
#include "map_system.hpp"
#include "map_init.hpp"
#include "animation_system.hpp"
#include "ui_system.hpp"
#include "ai_system.hpp"
#include <thread>

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem	    ai_system;
	WorldSystem     world_system;
	RenderSystem    renderer_system;
	AudioSystem     audio_system;
	PhysicsSystem   physics_system;
	InputSystem     input_system;
	PlayerSystem    player_system;
	MapSystem map_system;
	AnimationSystem animation_system;
	UISystem ui_system;

	registry.map_system = &map_system;
	registry.audio_system = &audio_system;
	registry.ui_system = &ui_system;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	renderer_system.init(window);
	audio_system.init();
	input_system.init(window);
	world_system.init(&renderer_system, &audio_system);
	map_system.init(&renderer_system);
	ai_system.init(&renderer_system, &audio_system);
	player_system.init(&renderer_system, &audio_system, &world_system, window);
	animation_system.init();
	ui_system.init(window, &world_system, &renderer_system, &audio_system);
	
	Entity fps_entity = Entity();
	UI& fps_ui = registry.uis.emplace(fps_entity);

	Motion& fps_motion = registry.motions.emplace(fps_entity);
	fps_motion.position = { 1800, 30 };
	fps_motion.scale = glm::vec2(.5f, .5f);

	Text& fps_counter = registry.texts.emplace(fps_entity);
	fps_counter.color = { 1.f, 1.f, 1.f };
	fps_counter.content = "";

	if (debugging.disable_v_sync) {
		glfwSwapInterval(0);
	}

	GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);
	float target_fps = (debugging.limit_fps > 0) ? debugging.limit_fps : video_mode->refreshRate;
	float render_interval = 1.f / target_fps;

	// variable timestep loop
	auto t = Clock::now();
	auto last_frame_time = t;
	auto last_render_time = t;
	int frame_count = 0;

	while (!world_system.is_over()) {
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		float time_since_last_second_s =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - last_frame_time)).count() / 1000000;
		frame_count++;
		if (time_since_last_second_s > 1.0) {
			float fps = frame_count / time_since_last_second_s;
			
			std::ostringstream fps_stream;
			fps_stream << std::fixed << std::setprecision(0) << fps;
			std::string fps_value = fps_stream.str();

			std::ostringstream ms_stream;
			ms_stream << std::fixed << std::setprecision(3) << elapsed_ms;
			std::string ms = ms_stream.str();

			std::string new_title = "Cyber-Yaga Vindicta " + fps_value + " FPS / " + ms + " ms";
			glfwSetWindowTitle(window, new_title.c_str());
			std::cout << "FPS: " << fps_value << " FPS / " << ms << " ms" << std::endl;

			//int fps_calc = std::min((int)fps, 60);
			fps_counter.content = "FPS: " + fps_value;

			last_frame_time = now;
			frame_count = 0;
		}


		// CK: be mindful of the order of your systems and rearrange this list only if necessary
		if (world_system.get_game_state() == GameState::TITLE_SCREEN) {
			player_system.step(elapsed_ms);
			animation_system.step(elapsed_ms);
			ui_system.step(elapsed_ms);
		}
		else {
			world_system.step(elapsed_ms);
			ai_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			player_system.step(elapsed_ms);
			animation_system.step(elapsed_ms);
			ui_system.step(elapsed_ms);
			world_system.handle_collisions(elapsed_ms);
			input_system.step();
		}

		if (std::chrono::duration<float>(t - last_render_time).count() >= render_interval) {
			renderer_system.draw();
			last_render_time = t;
		}
	}

	return EXIT_SUCCESS;
}