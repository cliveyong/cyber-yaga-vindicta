#include "animation_system.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"

// initialize animation system
void AnimationSystem::init() {
	// keep in case we need to initalize sprite sheets
	return;
}

// update texture in renderRequest
void AnimationSystem::update_render(Entity& entity, Animation& animation) {
	RenderRequest& rr = registry.renderRequests.get(entity);
	rr.used_texture = animation.textures[animation.state];
}

// progress the timers all animations
void AnimationSystem::progress_timers(float elapsed_ms) {
	for (auto& entity : registry.animations.entities) {
		Animation& animation = registry.animations.get(entity);
		if (animation.playing) {
			animation.counter_ms -= elapsed_ms;
		}
	}
}

// updates animation states
void AnimationSystem::update_animation() {
	for (auto& entity : registry.animations.entities) {
		Animation& animation = registry.animations.get(entity);
		if (animation.counter_ms <= 0.0f) {
			animation.state += 1;
			int current_state = animation.state % animation.textures.size();
			// remove animation if looping is false. Else the state goes back to 0
			if (current_state == 0) {
				if (!animation.looping) {
					registry.remove_all_components_of(entity);
					continue;
				}
				else {
					animation.state = current_state;
				}
			}

			update_render(entity, animation);
			animation.counter_ms = animation.change_time_ms;
		}
	}
}

void AnimationSystem::step(float elapsed_ms) {
	progress_timers(elapsed_ms);
	update_animation();
}
