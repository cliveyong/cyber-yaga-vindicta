#include "animation_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

// create an animation and specify if the animation is playing, looping, and the duration between animations
Entity create_animation(vec2 pos, vec2 vel, vec2 scale, float angle, std::vector<TEXTURE_ASSET_ID> textures, bool playing, bool looping, float change_ms, Z_INDEX z_index, bool is_ui_element, bool no_lighting) {
	Entity entity = Entity();
	Animation& animation = registry.animations.emplace(entity);
	animation.textures = textures;
	animation.playing = playing;
	animation.looping = looping;
	animation.counter_ms = change_ms;
	animation.change_time_ms = change_ms;

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.velocity = vel;
	motion.angle = angle;
	motion.scale = scale;

	registry.renderRequests.emplace(entity, RenderRequest{
		animation.textures[animation.state],
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		z_index,
		is_ui_element
	});

	if (is_ui_element) {
		registry.uis.emplace(entity);
	} else if (no_lighting) {
		registry.textureWithoutLighting.emplace(entity);
	}

	return entity;
}

void toggle_animation(Entity& entity, bool is_playing) {
	Animation& animation = registry.animations.get(entity);
	animation.playing = is_playing;
}

void toggle_looping(Entity& entity, bool is_looping) {
	Animation& animation = registry.animations.get(entity);
	animation.looping = is_looping;
}