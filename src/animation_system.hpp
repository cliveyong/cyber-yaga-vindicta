#pragma once
#include "common.hpp"
#include "tinyECS/entity.hpp"
#include "tinyECS/components.hpp"
#include <render_system.hpp>

class AnimationSystem {
public:
    void init();

    void update_animation();

    void step(float elapsed_ms);

    void progress_timers(float elapsed_ms);

    void update_render(Entity& entity, Animation& animation);

private:
};

