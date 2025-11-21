#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include <set>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{
	float health = STARTING_PLAYER_HEALTH;
	float speed = 300.0f;
	bool is_invincible = false;
};

// Melee component
struct Melee
{
	float counter_ms = 0.f;
	float melee_cooldown_ms = 800.f;
	float melee_hit_ms = 200.f;
	bool is_melee = false;
	bool melee_cooldown = false;
	float melee_range = 170.f;
	float melee_damage = 200.f;
};

// Dash component
struct Dash
{
	float dash_distance = 75.0f;
	float curr_distance = 0.0f;
	bool is_dashing = false;
	vec2 dash_velocity = {0.0f, 0.0f};
	float dash_speed = 1000.0f;
	float dash_cooldown_ms = 1000.0f;
	float cooldown_timer = 0.0f;
	float ghost_timer_ms = 0.f;
	float ghost_spawn_ms = 0.f;
	float ghost_duration = 250.f;
	float invincibility_timer_ms = 0.0f;
	float invincibility_duration = 500.0f;
};

struct RemoveTimer {
	float remaining_time = 250.f;
};

enum class GUN_TYPE {
	PISTOL = 0,
	ENEMY_PISTOL = PISTOL + 1,
	SHOTGUN = ENEMY_PISTOL + 1,
	DUMMY_GUN = SHOTGUN + 1,
	SMG = DUMMY_GUN + 1,
	RAILGUN = SMG + 1,
	REVOLVER = RAILGUN + 1,
	GUN_COUNT = REVOLVER + 1
};

// NPC/AI States 
enum class ENEMY_STATE {
	IDLE = 0,
	APPROACH = IDLE + 1,
	PURSUIT = APPROACH + 1,
	COMBAT = PURSUIT + 1,
	BACKOFF = COMBAT + 1,
	DEAD = BACKOFF + 1
};

// NPC/AI possible actions
enum class ENEMY_ACTION {
	ACTION_IDLE,		// Stand still and do nothing.
	ACTION_APPROACH,      // Move toward the player.
	ACTION_PURSUIT,		// Use A* pathfinding to move towards the player
	ACTION_COMBAT,	    // Shoot at the player.
	ACTION_BACKOFF
};


// NCP/AI Component
struct Enemy {
	ENEMY_STATE state = ENEMY_STATE::IDLE;
	float health = 200.f;
	float speed = 230.f;
	float pursuit_range = 700.f;
	float attack_range = 500.f;
	float backoff_range = 250.f;
	float stop_cooldown_ms = 0.f;
	float stop_timer = 350.f;
	float begin_shooting_timer = 500.f; // time to wait on reset
	float begin_shooting_ms = 500.f; // initial time to wait
	float turn_speed = 400.f;
};

struct EnemyBlueprint {
	ivec2 grid_position;
	GUN_TYPE gun_type = GUN_TYPE::PISTOL;
	float health = 100.f;
	float speed_factor = 5.f;
	float detection_range_factor = 6.6f;
	float attack_range_factor = 8.3f;
};

struct DeadEnemy {

};

struct TilesetInfo {
	int firstgid;
	std::string name;
};

// AI pathfinding component
struct PathComponent {
	std::vector<ivec2> waypoints;	// grid positions from A* pathfinding
	int current_index = 0;			// index into the waypoints vector
	bool valid = false;				// true if a valid path was found
	ivec2 target_cell = { -1, -1 };
};

struct TutorialEnemy {
	
};

// Projectile
struct Projectile {
	// Entity gun_source;
	int damage = 100.f;
	bool shot_by_player = false;
	bool is_gun = false;
	bool can_bounce = false;
	int remaining_penetrations = 0;
	std::set<int> hit_enemies;
	int ricochet_remaining = 0;
};

struct Reload {
	float remaining_time = 200.f;
	bool one_at_a_time = true;

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2  position = { 0, 0 };
	float angle_velocity = 0.f;
	float angle    = 0;
	vec2  velocity = { 0, 0 };
	vec2  scale    = { 10, 10 };
};

enum class ButtonType {
	START,
	RESUME,
	OPTIONS,
	QUIT
};

struct UIButton {
	vec2 position = { 0, 0 };
	vec2 scale = { 250, 80 };
	ButtonType type = ButtonType::START;
};

struct TitleScreenText {
	vec2 position = { 0,0 };
	vec2 scale = { 250, 80 };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

struct Debris {

};

// Structure to determine if entity is collidable
struct Collidable
{

};

// Structure that stores AABB components for collision
// collision_box is the bounding_box scale and the values go clockwise: {left, up, down, right}
struct AABB
{
	vec2 offset = {0.0f, 0.0f};
	vec2 collision_box = {0.0f, 0.0f};
};

// Structure that stores CircleBound Components for collision
// collision_radius is the radius of the collision circle
struct CircleBound
{
	vec2 offset = { 0.0f, 0.0f };
	float collision_radius = 0.0f;
};

// Structure that stores Mesh Components for collision
// collision_radius is the radius of the collision circle
struct meshCollidable
{
	std::vector<vec2> vertices;
	float offset = 0.0f;
	float collision_radius = 0.0f;
};

struct ShadowCaster {
	ivec2 start;
	ivec2 end;
};

// Data structure for toggling debug mode
struct Debug {
	int limit_fps = 0;
	bool disable_v_sync = false;
	bool wireframe = false;
	bool disable_fullscreen = false;
	bool disable_music = false;
	bool disable_text_rendering = false;
	bool disable_enemy_shooting = false;
	bool enable_button_outlines = false; // true = button outlines
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	vec3 ambient_color = TEAL_SUPERLIGHT;
	float glitch_remaining_ms = 0.f;
	float glitch_duration = 150.f;
	int resolution_x;
	int resolution_y;
	float grid_cell_size;
};

struct WalkSoundTimer
{
	float counter_ms = 0;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct Input
{
	std::unordered_map<int, bool> keys;
	std::unordered_map<int, bool> key_down_events;
	vec2 mouse_pos = { 0.f, 0.f };
};

struct Camera
{
	float speed = 1000.f;
	float max_distance = 500.f;
	float deadzone = 50.f;
	float dampening_offset = 100.f;
};

struct EntityHash {
    std::size_t operator()(const Entity& e) const {
        return std::hash<int>()(e.id()); 
    }
};

struct Laser
{
	// Note, an empty struct has size 1
};

struct StaticCollidable {

};

struct MovingCollidable {

};

struct MovingCircle {

};

struct MovingSAT {

};

struct Light {
	vec3 color;
	vec2 position;
	float radius;
	float intensity = 1.f;
	bool is_local = false;
	float height = 32.;
};

struct TextureWithoutLighting {
};

struct Character {
	GLuint TextureID;     
	glm::ivec2 Size;     
	glm::ivec2 Bearing; 
	GLuint Advance;
	char Character;
};

struct Text {
	std::string content;  // The actual string to be displayed
	glm::vec3 color;      // Color of the text (RGB)
};

enum class PICKUP_TYPE {
	GUN = 0,
	HEALTH_BOX = GUN + 1
};

struct Pickup {
	vec2 grid_pos;
	PICKUP_TYPE type = PICKUP_TYPE::GUN;
	GUN_TYPE gun_type = GUN_TYPE::PISTOL;
	int value = 0;
	float range = 60;
};


/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	PROJECTILE = 0,
	DEFAULT_NORMAL = PROJECTILE + 1,
	PLAYER_SHOOTING_PISTOL = DEFAULT_NORMAL + 1,
	PLAYER_SHOOTING_SHOTGUN = PLAYER_SHOOTING_PISTOL + 1,
	RIGHT_WALL1 = PLAYER_SHOOTING_SHOTGUN + 1,
	RIGHT_WALL1_NORMAL = RIGHT_WALL1 + 1,
	LEFT_WALL1 = RIGHT_WALL1_NORMAL + 1,
	LEFT_WALL1_NORMAL = LEFT_WALL1 + 1,
	TOP_WALL1 = LEFT_WALL1_NORMAL + 1,
	TOP_WALL1_NORMAL = TOP_WALL1 + 1,
	BOTTOM_WALL1 = TOP_WALL1_NORMAL + 1,
	BOTTOM_WALL1_NORMAL = BOTTOM_WALL1 + 1,
	TOP_LEFT_WALL1 = BOTTOM_WALL1_NORMAL + 1,
	TOP_LEFT_WALL1_NORMAL = TOP_LEFT_WALL1 + 1,
	TOP_RIGHT_WALL1 = TOP_LEFT_WALL1_NORMAL + 1,
	TOP_RIGHT_WALL1_NORMAL = TOP_RIGHT_WALL1 + 1,
	BOTTOM_LEFT_WALL1 = TOP_RIGHT_WALL1_NORMAL + 1,
	BOTTOM_LEFT_WALL1_NORMAL = BOTTOM_LEFT_WALL1 + 1,
	BOTTOM_RIGHT_WALL1 = BOTTOM_LEFT_WALL1_NORMAL + 1,
	BOTTOM_RIGHT_WALL1_NORMAL = BOTTOM_RIGHT_WALL1 + 1,
	BOTTOM_RIGHT_WALL_CORNER2 = BOTTOM_RIGHT_WALL1_NORMAL + 1,
	BOTTOM_RIGHT_WALL_CORNER2_NORMAL = BOTTOM_RIGHT_WALL_CORNER2 + 1,
	BOTTOM_LEFT_WALL_CORNER2 = BOTTOM_RIGHT_WALL_CORNER2_NORMAL + 1,
	BOTTOM_LEFT_WALL_CORNER2_NORMAL = BOTTOM_LEFT_WALL_CORNER2 + 1,
	TOP_RIGHT_WALL_CORNER2 = BOTTOM_LEFT_WALL_CORNER2_NORMAL + 1,
	TOP_RIGHT_WALL_CORNER2_NORMAL = TOP_RIGHT_WALL_CORNER2 + 1,
	TOP_LEFT_WALL_CORNER2 = TOP_RIGHT_WALL_CORNER2_NORMAL + 1,
	TOP_LEFT_WALL_CORNER2_NORMAL = TOP_LEFT_WALL_CORNER2 + 1,
	BOTTOM_RIGHT_WALL_CORNER1 = TOP_LEFT_WALL_CORNER2_NORMAL + 1,
	BOTTOM_RIGHT_WALL_CORNER_NORMAL = BOTTOM_RIGHT_WALL_CORNER1 + 1,
	BOTTOM_LEFT_WALL_CORNER1 = BOTTOM_RIGHT_WALL_CORNER_NORMAL + 1,
	BOTTOM_LEFT_WALL_CORNER_NORMAL = BOTTOM_LEFT_WALL_CORNER1 + 1,
	TOP_RIGHT_WALL_CORNER1 = BOTTOM_LEFT_WALL_CORNER_NORMAL + 1,
	TOP_RIGHT_WALL_CORNER_NORMAL = TOP_RIGHT_WALL_CORNER1 + 1,
	TOP_LEFT_WALL_CORNER1 = TOP_RIGHT_WALL_CORNER_NORMAL + 1,
	TOP_LEFT_WALL_CORNER_NORMAL = TOP_LEFT_WALL_CORNER1 + 1,
	FLOOR_1 = TOP_LEFT_WALL_CORNER_NORMAL + 1,
	FLOOR_1_NORMAL = FLOOR_1 + 1,
	FLOOR_2 = FLOOR_1_NORMAL + 1,
	FLOOR_2_NORMAL = FLOOR_2 + 1,
	FLOOR_3 = FLOOR_2_NORMAL + 1,
	FLOOR_4 = FLOOR_3 + 1,
	FLOOR_5 = FLOOR_4 + 1,
	HOLE = FLOOR_5 + 1,
	LASER = HOLE + 1,
	ENEMY = LASER + 1,
	DEAD_ENEMY = ENEMY + 1,
	CROSSHAIR = DEAD_ENEMY + 1,
	HEALTH_BORDER = CROSSHAIR + 1,
	HEALTH_0 = HEALTH_BORDER + 1,
	HEALTH_1 = HEALTH_0 + 1,
	HEALTH_2 = HEALTH_1 + 1,
	HEALTH_3 = HEALTH_2 + 1,
	HEALTH_4 = HEALTH_3 + 1,
	HEALTH_5 = HEALTH_4 + 1,
	HEALTH_6 = HEALTH_5 + 1,
	TITLE_SCREEN = HEALTH_6 + 1,
	CINEMAITC_CUTSCENE_0 = TITLE_SCREEN + 1,
	CINEMAITC_CUTSCENE_1 = CINEMAITC_CUTSCENE_0 + 1,
	CINEMAITC_CUTSCENE_2 = CINEMAITC_CUTSCENE_1 + 1,
	CINEMAITC_CUTSCENE_3 = CINEMAITC_CUTSCENE_2 + 1,
	CINEMAITC_CUTSCENE_4 = CINEMAITC_CUTSCENE_3 + 1,
	CINEMAITC_CUTSCENE_5 = CINEMAITC_CUTSCENE_4 + 1,
	CINEMAITC_CUTSCENE_6 = CINEMAITC_CUTSCENE_5 + 1,
	CINEMAITC_CUTSCENE_7 = CINEMAITC_CUTSCENE_6 + 1,
	CINEMAITC_CUTSCENE_8 = CINEMAITC_CUTSCENE_7 + 1,
	START_TEXT = CINEMAITC_CUTSCENE_8 + 1,
	RESUME_TEXT = START_TEXT + 1,
	OPTIONS_TEXT = RESUME_TEXT + 1,
	QUIT_TEXT = OPTIONS_TEXT + 1,
	BUTTON_TESTER = QUIT_TEXT + 1,
	STAMINA_0 = BUTTON_TESTER + 1,
	STAMINA_1 = STAMINA_0 + 1,
	STAMINA_2 = STAMINA_1 + 1,
	STAMINA_3 = STAMINA_2 + 1,
	STAMINA_4 = STAMINA_3 + 1,
	STAMINA_5 = STAMINA_4 + 1,
	STAMINA_6 = STAMINA_5 + 1,
	STAMINA_7 = STAMINA_6 + 1,
	STAMINA_8 = STAMINA_7 + 1,
	STAMINA_9 = STAMINA_8 + 1,
	STAMINA_10 = STAMINA_9 + 1,
	STAMINA_11 = STAMINA_10 + 1,
	STAMINA_12 = STAMINA_11 + 1,
	STAMINA_13 = STAMINA_12 + 1,
	STAMINA_14 = STAMINA_13 + 1,
	STAMINA_15 = STAMINA_14 + 1,
	STAMINA_16 = STAMINA_15 + 1,
	MUZZLE_FLASH0 = STAMINA_16 + 1,
	MUZZLE_FLASH1 = MUZZLE_FLASH0 + 1,
	MUZZLE_FLASH2 = MUZZLE_FLASH1 + 1,
	WALL_PARTICLE_1 = MUZZLE_FLASH2 + 1,
	WALL_PARTICLE_2 = WALL_PARTICLE_1 + 1,
	WALL_PARTICLE_3 = WALL_PARTICLE_2 + 1,
	WALL_PARTICLE_4 = WALL_PARTICLE_3 + 1,
	WALL_PARTICLE_5 = WALL_PARTICLE_4 + 1,
	BLOOD_SPLATTER_0 = WALL_PARTICLE_5 + 1,
	BLOOD_SPLATTER_1 = BLOOD_SPLATTER_0 + 1,
	BLOOD_SPLATTER_2 = BLOOD_SPLATTER_1 + 1,
	BLOOD_SPLATTER_3 = BLOOD_SPLATTER_2 + 1,
	BLOOD_SPLATTER_4 = BLOOD_SPLATTER_3 + 1,
	BLOOD_SPLATTER_5 = BLOOD_SPLATTER_4 + 1,
	BLOOD_SPLATTER_6 = BLOOD_SPLATTER_5 + 1,
	BLOOD_SPLATTER_7 = BLOOD_SPLATTER_6 + 1,
	DAMAGE_INDICATOR_0 = BLOOD_SPLATTER_7 + 1,
	DAMAGE_INDICATOR_1 = DAMAGE_INDICATOR_0 + 1,
	DAMAGE_INDICATOR_2 = DAMAGE_INDICATOR_1 + 1,
	DAMAGE_INDICATOR_3 = DAMAGE_INDICATOR_2 + 1,
	INSTRUCTION_WASD = DAMAGE_INDICATOR_3 + 1,
	INSTRUCTION_LEFT_CLICK = INSTRUCTION_WASD + 1,
	INSTRUCTION_SPACE = INSTRUCTION_LEFT_CLICK + 1,
	INSTRUCTION_R = INSTRUCTION_SPACE + 1,
	INSTRUCTION_HEALTH_PACK = INSTRUCTION_R + 1,
	INSTRUCTION_DODGE = INSTRUCTION_HEALTH_PACK + 1,
	INSTRUCTION_MELEE = INSTRUCTION_DODGE + 1,
	CONTAINER = INSTRUCTION_MELEE + 1,
	CONTAINER_DISABLED = CONTAINER + 1,
	SERVER_DISABLED = CONTAINER_DISABLED + 1,
	SERVER = SERVER_DISABLED + 1,
	SERVER_DOWN = SERVER + 1,
	SERVER_NORMAL = SERVER_DOWN + 1,
	CHEST_BASE = SERVER_NORMAL + 1,
	CHEST_NORMAL = CHEST_BASE + 1,
	CHEST_OPEN = CHEST_NORMAL + 1,
	CHEST_OPEN_NORMAL = CHEST_OPEN + 1,
	CHEST0 = CHEST_OPEN_NORMAL + 1,
	CHEST0_DISABLED = CHEST0 + 1,
	CHEST0_ENABLED = CHEST0_DISABLED + 1,
	RED_LIGHT = CHEST0_ENABLED + 1,
	RED_LIGHT_END = RED_LIGHT + 1,
	BLUE_LIGHT = RED_LIGHT_END + 1,
	BLUE_LIGHT_END = BLUE_LIGHT + 1,
	HEALTH_BOX = BLUE_LIGHT_END + 1,
	PISTOL_PICKUP = HEALTH_BOX + 1,
	SMG_PICKUP = PISTOL_PICKUP + 1,
	SHOTGUN_PICKUP = SMG_PICKUP + 1,
	RAILGUN_PICKUP = SHOTGUN_PICKUP + 1,
	REVOLVER_PICKUP = RAILGUN_PICKUP + 1,
	PISTOL = REVOLVER_PICKUP + 1,
	SLASH_0 = PISTOL + 1,
	SLASH_1 = SLASH_0 + 1,
	SLASH_2 = SLASH_1 + 1,
	SLASH_3 = SLASH_2 + 1,
	SHOTGUN_UI = SLASH_3 + 1,
	SMG_UI = SHOTGUN_UI + 1,
	LEVEL_0_TEXT = SMG_UI + 1,
	LEVEL_1_TEXT = LEVEL_0_TEXT + 1,
	LEVEL_2_TEXT = LEVEL_1_TEXT + 1,
	LEVEL_3_TEXT = LEVEL_2_TEXT + 1,
	LEVEL_4_TEXT = LEVEL_3_TEXT + 1,
	GAME_END_TEXT = LEVEL_4_TEXT + 1,
	DOOR = GAME_END_TEXT + 1,
	DOOR_NORMAL = DOOR + 1,
	DOOR_LEFT =	DOOR_NORMAL	+ 1,
	DOOR_RIGHT = DOOR_LEFT + 1,
	DOOR_TOP = DOOR_RIGHT + 1,
	DOOR_LEFT_NORMAL = DOOR_TOP + 1,
	DOOR_RIGHT_NORMAL = DOOR_LEFT_NORMAL + 1,
	DOOR_TOP_NORMAL = DOOR_RIGHT_NORMAL + 1,
	DOOR_PARTICLE_0 = DOOR_TOP_NORMAL + 1,
	DOOR_PARTICLE_1 = DOOR_PARTICLE_0 + 1,
	DOOR_PARTICLE_2 = DOOR_PARTICLE_1 + 1,
	DOOR_PARTICLE_3 = DOOR_PARTICLE_2 + 1,
	DOOR_PARTICLE_4 = DOOR_PARTICLE_3 + 1,
	DOOR_PARTICLE_5 = DOOR_PARTICLE_4 + 1,
	DOOR_PARTICLE_6 = DOOR_PARTICLE_5 + 1,
	DOOR_PARTICLE_7 = DOOR_PARTICLE_6 + 1,
	DOOR_PARTICLE_8 = DOOR_PARTICLE_7 + 1,
	DOOR_PARTICLE_9 = DOOR_PARTICLE_8 + 1,
	RAILGUN_UI = DOOR_PARTICLE_9 + 1,
	REVOLVER_UI = RAILGUN_UI + 1,
	TEXTURE_COUNT = REVOLVER_UI + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	TEXTURED = 0,
	VIGNETTE = TEXTURED + 1,
	LIGHT = VIGNETTE + 1,
	MULTIPLY = LIGHT + 1,
	SHADOWS = MULTIPLY + 1,
	TEXTURED_WITH_NORMAL = SHADOWS + 1,
	FONT = TEXTURED_WITH_NORMAL + 1,
	EFFECT_COUNT = FONT + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class SOUND_ASSET_ID {
	PISTOL_GUNSHOT_1 = 0,
	FOOTSTEP = PISTOL_GUNSHOT_1 + 1,
	BULLET_HIT_WALL_1 = FOOTSTEP + 1,
	BULLET_HIT_FLESH_1 = BULLET_HIT_WALL_1 + 1,
	DODGE_WOOSH = BULLET_HIT_FLESH_1 + 1,
	DASH = DODGE_WOOSH + 1,
	PLAYER_HIT_1 = DASH + 1,
	ENEMY_HIT_1 = PLAYER_HIT_1 + 1,
	ENEMY_HIT_2 = ENEMY_HIT_1 + 1,
	ENEMY_HIT_3 = ENEMY_HIT_2 + 1,
	ENEMY_HIT_4 = ENEMY_HIT_3 + 1,
	ENEMY_HIT_5 = ENEMY_HIT_4 + 1,
	ENEMY_HIT_6 = ENEMY_HIT_5 + 1,
	ENEMY_HIT_7 = ENEMY_HIT_6 + 1,
	ENEMY_HIT_8 = ENEMY_HIT_7 + 1,
	BULLET_FLYBY_1 = ENEMY_HIT_8 + 1,
	BULLET_FLYBY_2 = BULLET_FLYBY_1 + 1,
	BULLET_FLYBY_3 = BULLET_FLYBY_2 + 1,
	PISTOL_DRY_FIRE_1 = BULLET_FLYBY_3 + 1,
	PISTOL_RELOAD_1 = PISTOL_DRY_FIRE_1 + 1,
	PISTOL_LOAD_1 = PISTOL_RELOAD_1 + 1,
	PISTOL_LOAD_2 = PISTOL_LOAD_1 + 1,
	PISTOL_LOAD_3 = PISTOL_LOAD_2 + 1,
	PISTOL_LOAD_4 = PISTOL_LOAD_3 + 1,
	PISTOL_LOAD_5 = PISTOL_LOAD_4 + 1,
	HEALTH_BOOST = PISTOL_LOAD_5 + 1,
	SLASH = HEALTH_BOOST + 1,
	SHOTGUN_SHOT_1 = SLASH + 1,
	DOOR_BREAKING = SHOTGUN_SHOT_1 + 1,
	REVOLVER_DRY_FIRE_1 = DOOR_BREAKING + 1,
	REVOLVER_SHOT_1 = REVOLVER_DRY_FIRE_1 + 1,
	REVOLVER_RELOAD_1 = REVOLVER_SHOT_1 + 1,
	REVOLVER_COCK_1 = REVOLVER_RELOAD_1 + 1,
	SHOTGUN_RELOAD_1 = REVOLVER_COCK_1 + 1,
	SHOTGUN_COCK_1 = SHOTGUN_RELOAD_1 + 1,
	RAILGUN_RELOAD_1 = SHOTGUN_COCK_1 + 1,
	RAILGUN_SHOT_1 = RAILGUN_RELOAD_1 + 1,
	SOUND_COUNT = RAILGUN_SHOT_1 + 1
};
constexpr int sound_count =  static_cast<int>(SOUND_ASSET_ID::SOUND_COUNT);

// Gun
struct Gun {
	GUN_TYPE gun_type = GUN_TYPE::PISTOL;
	float projectile_speed = 3000.0f;
	float damage = 100.f;
	float cooldown_ms = 250.0f;
	float cooldown_timer_ms = 0.0f;
	float reload_time = 1050.0f;
	int magazine_size = 4;
	int remaining_bullets = 14;
	int current_magazine = 4;
	float throw_timer_ms = 0.f;
	float throw_cooldown_time = 500.f;
	int projectile_count = 1;
	float spread_cone = 15.f;
	float recoil_pushback = 1.7f;
	int penetrating_count = 0;
	int ricochet_count = 0;
	bool reload_one_at_a_time = false;
	SOUND_ASSET_ID sound_effect = SOUND_ASSET_ID::PISTOL_GUNSHOT_1;
	SOUND_ASSET_ID click_effect = SOUND_ASSET_ID::PISTOL_DRY_FIRE_1;
	SOUND_ASSET_ID reload_effect = SOUND_ASSET_ID::PISTOL_RELOAD_1;
	SOUND_ASSET_ID rack_effect = SOUND_ASSET_ID::PISTOL_RELOAD_1;
	TEXTURE_ASSET_ID hud_sprite = TEXTURE_ASSET_ID::PISTOL;
	TEXTURE_ASSET_ID thrown_sprite = TEXTURE_ASSET_ID::PISTOL;
};

enum class MUSIC_ASSET_ID {
	MUSIC1 = 0,
	MUSIC2 = MUSIC1 + 1,
	MUSIC3 = MUSIC2 + 1,
	MUSIC4 = MUSIC3 + 1,
	MUSIC_COUNT = MUSIC4 + 1
};
constexpr int music_count = static_cast<int>(MUSIC_ASSET_ID::MUSIC_COUNT);

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	DEBUG_LINE = SPRITE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	SHADOW_QUAD = SCREEN_TRIANGLE + 1,
	TEXT = SHADOW_QUAD + 1,
	GEOMETRY_COUNT = TEXT + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class Z_INDEX {
	FLOOR = 0,
	DEAD_ENEMY = FLOOR + 1,
	PROP = DEAD_ENEMY + 1,
	PICKUP = PROP + 1,
	PLAYER_DASH = PICKUP + 1,
	LASER = PLAYER_DASH + 1,
	PLAYER = LASER + 1,
	BLOOD_SPLATTER = PLAYER + 1,
	PROJECTILE = BLOOD_SPLATTER + 1,
	WALL_PARTICLE = PROJECTILE + 1,
	MUZZLE_FLASH = WALL_PARTICLE + 1,
	WALL = MUZZLE_FLASH + 1,
	NO_LIGHTING = WALL + 1,
	HUD = NO_LIGHTING + 1,
	TITLE_SCREEN = HUD + 1,
	TITLE_SCREEN_BUTTON = TITLE_SCREEN + 1,
	CROSSHAIR = TITLE_SCREEN_BUTTON + 1,
	CUTSCENE = CROSSHAIR + 1,
	ALWAYS_ON_TOP = CUTSCENE + 1,
	Z_INDEX_COUNT = ALWAYS_ON_TOP + 1
};
const int z_index_count = (int)Z_INDEX::Z_INDEX_COUNT;

enum class TEXTURE_ORIGIN_ID {
	FILE = 0,
	MAP = 1,
	TEXTURE_ORIGIN_COUNT = MAP + 1
};
const int texture_origin_count = (int)TEXTURE_ORIGIN_ID::TEXTURE_ORIGIN_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID   used_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect   = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	Z_INDEX			   z_index = Z_INDEX::ALWAYS_ON_TOP;
	bool is_ui_element = false;
	TEXTURE_ORIGIN_ID  texture_origin = TEXTURE_ORIGIN_ID::FILE;
	TEXTURE_ASSET_ID   texture_normal  = TEXTURE_ASSET_ID::DEFAULT_NORMAL;
	float alpha = 1.f;
	float outline_transparency = 0.f;
	vec3 outline_color_a = {0., 0., 0.};
	vec3 outline_color_b = {0., 0., 0.};
};

enum class TILE_ID {
	EMPTY = 0,
	WALL = EMPTY + 1,
	FLOOR = WALL + 1,
	OPEN_DOOR = FLOOR + 1,
	CLOSED_DOOR = OPEN_DOOR + 1,
	TILE_COUNT = CLOSED_DOOR + 1
};

struct Animation {
	int state = 0;
	float counter_ms;
	float change_time_ms;
	bool playing;
	bool looping;
	std::vector<TEXTURE_ASSET_ID> textures;
};

enum class WALL_DIRECTION {
	TOP = 0,
	BOTTOM = TOP + 1,
	LEFT = BOTTOM + 1,
	RIGHT = LEFT + 1,
	BOTTOM_RIGHT = RIGHT + 1,
	BOTTOM_LEFT = BOTTOM_RIGHT + 1,
	TOP_RIGHT = BOTTOM_LEFT + 1,
	TOP_LEFT = TOP_RIGHT + 1,
	NONE = TOP_LEFT + 1
};

struct ivec2_hash {
    std::size_t operator()(const ivec2& v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

struct vec2_hash {
    std::size_t operator()(const vec2& v) const {
        // Hash both floats and combine them
        std::size_t h1 = std::hash<float>()(v.x);
        std::size_t h2 = std::hash<float>()(v.y);
        return h1 ^ (h2 << 1);
    }
};

struct Prop {
	TEXTURE_ASSET_ID texture;
	TEXTURE_ASSET_ID normal;
	vec2 size;
	bool collidable;
	vec2 collision_offset = {0, 0};
	vec2 collision_size = {0, 0};
	float angle = 0;
	bool destroyable = false;
	bool vertical = false;
	int prop_size = 1;
};

struct Tile {
	TEXTURE_ASSET_ID texture;
	TEXTURE_ASSET_ID normal = TEXTURE_ASSET_ID::DEFAULT_NORMAL;
	TILE_ID id;
	ivec2 size;
};

struct Room
{
	int room_height;
	int room_width;
	std::unordered_map<int, Tile> room_tiles;
	std::vector<std::vector<int>> room_grid;
	std::vector<std::vector<WALL_DIRECTION>> wall_directions;
	std::unordered_map<vec2, Prop, ivec2_hash> props;
};

struct Map {
	int grid_height;
	int grid_width;
	std::vector<std::vector<TILE_ID>> tile_id_grid; // Stored the TILE_ID of each tile in the grid, needed for fast LOS approximation
	std::vector<std::vector<int>> tile_object_grid; // Stores the id of the Tile object in each tile of the grid
	std::vector<std::vector<int>> room_mask; // Stores the room id of each tile in the grid
	std::vector<ivec2> prop_doors_list;
	std::vector<std::pair<Entity, std::vector<Entity>>> prop_doors_pair;
	std::unordered_map<vec2, Entity, ivec2_hash> prop_doors; // Stores all the location of doors
	std::unordered_map<int, Tile> tiles; // Stores the Tile object themselves
	std::vector<std::vector<WALL_DIRECTION>> wall_directions;
	std::vector<ivec2> door_locations;
	std::vector<Entity> rendered_entities;
	std::vector<Light> lights;
	std::unordered_map<vec2, Prop, ivec2_hash> props;
	std::unordered_map<vec2, Prop, ivec2_hash> information_props;
	std::vector<Pickup> pickups;
	std::vector<EnemyBlueprint> enemy_blueprints;
	GLuint map_texture = 0;
	GLuint map_normal = 0;
	vec2 start_location;
	MUSIC_ASSET_ID music = MUSIC_ASSET_ID::MUSIC1;
	GUN_TYPE gun_type = GUN_TYPE::PISTOL;
	TEXTURE_ASSET_ID level_title = TEXTURE_ASSET_ID::LEVEL_0_TEXT;
};

struct GameProgress {
	int level = 0;
};

struct UI {
	int state;
	std::vector<TEXTURE_ASSET_ID> textures;
};

struct InstructionMessage {

};

struct SpatialHash {
	float cell_size = 100.f;
	int height;
	int width;
	std::vector<std::vector < std::vector<Entity>>> grid;
};
