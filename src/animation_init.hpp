#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"

Entity create_animation(vec2 pos, vec2 vel, vec2 scale, float angle, std::vector<TEXTURE_ASSET_ID> textures, bool playing, bool looping, float change_ms, Z_INDEX z_index, bool is_ui_element = false, bool no_lighting = false);

void toggle_animation(Entity& entity, bool is_playing);

void toggle_looping(Entity& entity, bool is_looping);
