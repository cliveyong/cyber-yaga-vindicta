#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

class InputSystem
{
public:
    InputSystem();
    ~InputSystem();

    void step();

    void update_input();
    
    void init(GLFWwindow* window);

private:
    GLFWwindow* window;
    Input& inputs;
    
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button_pressed(int button, int action, int mods);
};
