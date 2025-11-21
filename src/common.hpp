#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};

// game constants
const int GRID_HEIGHT = 18;

const int WALK_SOUND_TIMER_MS = 400;	// number of milliseconds between footstep sounds
const int DASH_GHOST_TIMER_MS = 30;
const float DASH_GHOST_DURATION_TIMER_MS = 350;
const float SHIELD_TRANSITION_TIME_MS = 100;

const int PROJECTILE_DAMAGE = 10;
const float STARTING_PLAYER_HEALTH = 600.f;

const vec3 MAGENTA_DARK = vec3(0.10f, 0.03f, 0.20f);
const vec3 TEAL_LIGHT = vec3(0.5, 0.5, 0.5);
const vec3 TEAL_SUPERLIGHT = vec3(0.57, 0.57, 0.57);
const vec3 DEEP_BLUE = vec3(0.05f, 0.15f, 0.25f);
const vec3 COOL_NEON = { 0.f, 0.7f, 1.f };
const vec3 HOT_PINK = { 1.0f, 0.1f, 0.6f };
const vec3 ACID_GREEN = { 0.3f, 1.0f, 0.3f };
const vec3 AMBER_ORANGE = { 1.0f, 0.5f, 0.1f };
const vec3 DEEP_RED = { 1.0f, 0.2f, 0.2f };


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();

void print_2d_vector(std::vector<std::vector<int>>& vec);

vec2 lerp(vec2 start, vec2 end, float time);

int get_rand(int range_start, int range_end);
