#include "common.hpp"
#include "tinyECS/components.hpp"

const Prop CHEST_BASE = Prop{ 
	TEXTURE_ASSET_ID::CHEST_BASE,
	TEXTURE_ASSET_ID::CHEST_NORMAL,
	{1, 1},
	true,
	{0, 0.1},
	{0.9, 0.7}
	//{0, 0.3}, // TODO: Try these later after we fix collisions
	//{0.95, 0.4}
};

const Prop CHEST0 = Prop{ 
	TEXTURE_ASSET_ID::CHEST0,
	TEXTURE_ASSET_ID::CHEST_NORMAL,
	{1, 1},
	true,
	{0, 0.1},
	{0.9, 0.7}
};

const Prop CHEST_ENABLED = Prop{ 
	TEXTURE_ASSET_ID::CHEST0_ENABLED,
	TEXTURE_ASSET_ID::CHEST_NORMAL,
	{1, 1},
	true,
	{0, 0.1},
	{0.9, 0.7}
};

const Prop CHEST_DISABLED = Prop{ 
	TEXTURE_ASSET_ID::CHEST0_DISABLED,
	TEXTURE_ASSET_ID::CHEST_NORMAL,
	{1, 1},
	true,
	{0, 0.1},
	{0.9, 0.7}
};

const Prop CHEST_OPEN = Prop{ 
	TEXTURE_ASSET_ID::CHEST_OPEN,
	TEXTURE_ASSET_ID::CHEST_OPEN_NORMAL,
	{1, 1},
	true,
	{0, 0.1},
	{0.9, 0.7}
};

const Prop CONTAINER = Prop{
	TEXTURE_ASSET_ID::CONTAINER,
	TEXTURE_ASSET_ID::SERVER_NORMAL,
	{0.5, 1},
	true,
	{0, 0},
	{0.5, 1}
};

const Prop CONTAINER_DISABLED = Prop{
	TEXTURE_ASSET_ID::CONTAINER_DISABLED,
	TEXTURE_ASSET_ID::SERVER_NORMAL,
	{0.5, 1},
	true,
	{0, 0},
	{0.5, 1}
};

const Prop SERVER = Prop{
	TEXTURE_ASSET_ID::SERVER,
	TEXTURE_ASSET_ID::SERVER_NORMAL,
	{0.5, 1},
	true,
	{0, 0},
	{0.5, 1}
};

const Prop SERVER_DOWN = Prop{
	TEXTURE_ASSET_ID::SERVER_DOWN,
	TEXTURE_ASSET_ID::SERVER_NORMAL,
	{0.5, 1},
	true,
	{0, 0},
	{0.5, 1}
};

const Prop SERVER_DISABLED = Prop{
	TEXTURE_ASSET_ID::SERVER_DISABLED,
	TEXTURE_ASSET_ID::SERVER_NORMAL,
	{0.5, 1},
	true,
	{0, 0},
	{0.5, 1}
};

const Prop WIDE_DOOR_HOR = Prop{
	TEXTURE_ASSET_ID::DOOR,
	TEXTURE_ASSET_ID::DOOR_NORMAL,
	{8,1.4},
	true,
	{0, -.6},
	{4, 2},
	0,
	true,
	false,
	2
};

const Prop WIDE_DOOR_VER = Prop{
	TEXTURE_ASSET_ID::DOOR,
	TEXTURE_ASSET_ID::DOOR_NORMAL,
	{1.4,8},
	true,
	{0, -.6},
	{4, 2},
	0,
	true,
	true,
	2
};

const Prop NARROW_DOOR_HOR = Prop{
	TEXTURE_ASSET_ID::DOOR,
	TEXTURE_ASSET_ID::DOOR_NORMAL,
	{5.1,1.3},
	true,
	{0, -0.6},
	{2, 2},
	true
};

const Prop NARROW_DOOR_VER = Prop{
	TEXTURE_ASSET_ID::DOOR,
	TEXTURE_ASSET_ID::DOOR_NORMAL,
	{1.4,4.7},
	true,
	{0, -.6},
	{2, 2},
	0,
	true,
	true
};

const Prop BLUE_LIGHT_VERTICAL = Prop{
	TEXTURE_ASSET_ID::BLUE_LIGHT,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	90.f
};

const Prop BLUE_LIGHT_TOP = Prop{
	TEXTURE_ASSET_ID::BLUE_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	90.f
};

const Prop BLUE_LIGHT_BOTTOM = Prop{
	TEXTURE_ASSET_ID::BLUE_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	-90.f
};

const Prop BLUE_LIGHT = Prop{
	TEXTURE_ASSET_ID::BLUE_LIGHT,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	0.f
};

const Prop BLUE_LIGHT_LEFT = Prop{
	TEXTURE_ASSET_ID::BLUE_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	0.f
};

const Prop BLUE_LIGHT_RIGHT = Prop{
	TEXTURE_ASSET_ID::BLUE_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	180.f
};

const Prop RED_LIGHT_VERTICAL = Prop{
	TEXTURE_ASSET_ID::RED_LIGHT,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	90.f
};

const Prop RED_LIGHT_TOP = Prop{
	TEXTURE_ASSET_ID::RED_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	90.f
};

const Prop RED_LIGHT_BOTTOM = Prop{
	TEXTURE_ASSET_ID::RED_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	-90.f
};

const Prop RED_LIGHT_HORIZONTAL = Prop{
	TEXTURE_ASSET_ID::RED_LIGHT,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	0.f
};

const Prop RED_LIGHT_LEFT = Prop{
	TEXTURE_ASSET_ID::RED_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	0.f
};

const Prop RED_LIGHT_RIGHT = Prop{
	TEXTURE_ASSET_ID::RED_LIGHT_END,
	TEXTURE_ASSET_ID::DEFAULT_NORMAL,
	{1, 1},
	false,
	{0, 0},
	{0, 0},
	180.f
};

const std::unordered_map<int, Prop> PROPS_AND_OTHERS_ID_MAP = {
	{0, CHEST_BASE},
	{1, CHEST_OPEN},
	{2, CHEST0},
	{3, CHEST_DISABLED},
	{4, CHEST_ENABLED},
	{5, CONTAINER},
	{6, CONTAINER_DISABLED},
	{11, SERVER},
	{13, SERVER_DISABLED},
	{14, SERVER_DOWN}
};

const std::unordered_map<int, Prop> DOOR_FRAME_AND_LIGHTS_ID_MAP = {
	{40, RED_LIGHT_TOP},
	{47, BLUE_LIGHT_TOP},
	{48, RED_LIGHT_VERTICAL},
	{55, BLUE_LIGHT_VERTICAL},
	{56, RED_LIGHT_BOTTOM},
	{57, RED_LIGHT_LEFT},
	{58, RED_LIGHT_HORIZONTAL},
	{59, RED_LIGHT_RIGHT},
	{60, BLUE_LIGHT_LEFT},
	{61, BLUE_LIGHT},
	{62, BLUE_LIGHT_RIGHT},
	{63, BLUE_LIGHT_BOTTOM}
};
