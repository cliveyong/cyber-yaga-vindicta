#include "input_system.hpp"

#include <iostream>

InputSystem::InputSystem() :
    inputs(registry.inputs.emplace(Entity()))
{
	inputs.keys.emplace(GLFW_KEY_ESCAPE, false);
	inputs.keys.emplace(GLFW_KEY_W, false);
	inputs.keys.emplace(GLFW_KEY_A, false);
	inputs.keys.emplace(GLFW_KEY_S, false);
	inputs.keys.emplace(GLFW_KEY_D, false);
	inputs.keys.emplace(GLFW_KEY_SPACE, false);
	inputs.keys.emplace(GLFW_MOUSE_BUTTON_LEFT, false);
	inputs.keys.emplace(GLFW_MOUSE_BUTTON_RIGHT, false);
	inputs.keys.emplace(GLFW_KEY_P, false);
	inputs.keys.emplace(GLFW_KEY_R, false);
	inputs.keys.emplace(GLFW_KEY_F, false);
	inputs.keys.emplace(GLFW_KEY_LEFT_SHIFT, false);
	inputs.keys.emplace(GLFW_KEY_ENTER, false);

	inputs.key_down_events.emplace(GLFW_MOUSE_BUTTON_LEFT, false);
	inputs.key_down_events.emplace(GLFW_MOUSE_BUTTON_RIGHT, false);
	inputs.key_down_events.emplace(GLFW_KEY_SPACE, false);
	inputs.key_down_events.emplace(GLFW_KEY_ENTER, false);
	inputs.key_down_events.emplace(GLFW_KEY_R, false);
}

void InputSystem::step() {
	for (const auto& key_down_event : inputs.key_down_events) {
		inputs.key_down_events[key_down_event.first] = false;
    }
}

InputSystem::~InputSystem() {
    registry.inputs.clear();
}

void InputSystem::init(GLFWwindow* window) {
    this->window = window;

	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((InputSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((InputSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((InputSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };
	
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);
}

void InputSystem::on_key(int key, int, int action, int mod) {
	if (inputs.keys.find(key) == inputs.keys.end()) {
		return;
	}

	if (action == GLFW_PRESS) {
		inputs.keys[key] = true;
		inputs.key_down_events[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		inputs.keys[key] = false;
	}

}

void InputSystem::on_mouse_move(vec2 mouse_position) {
	inputs.mouse_pos = mouse_position;
}

void InputSystem::on_mouse_button_pressed(int button, int action, int mods) {
	if (inputs.keys.find(button) == inputs.keys.end()) {
		return;
	}

	if (action == GLFW_PRESS) {
		inputs.keys[button] = true;
		inputs.key_down_events[button] = true;
	}
	else if (action == GLFW_RELEASE) {
		inputs.keys[button] = false;
	}
}
