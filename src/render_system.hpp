#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count>  texture_dimensions;

	mat3 ortho_matrix;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
	};

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("projectiles/bullet.png"),
		textures_path("default_normal.png"),
		textures_path("player/player_shooting_pistol.png"),
		textures_path("player/player_shooting_shotgun.png"),
		textures_path("tiles/walls/right_wall.png"),
		textures_path("tiles/walls/right_wall_normal.png"),
		textures_path("tiles/walls/left_wall.png"),
		textures_path("tiles/walls/left_wall_normal.png"),
		textures_path("tiles/walls/top_wall.png"),
		textures_path("tiles/walls/top_wall_normal.png"),
		textures_path("tiles/walls/bottom_wall.png"),
		textures_path("tiles/walls/bottom_wall_normal.png"),
		textures_path("tiles/walls/top_left_wall.png"),
		textures_path("tiles/walls/top_left_wall_normal.png"),
		textures_path("tiles/walls/top_right_wall.png"),
		textures_path("tiles/walls/top_right_wall_normal.png"),
		textures_path("tiles/walls/bottom_left_wall.png"),
		textures_path("tiles/walls/bottom_left_wall_normal.png"),
		textures_path("tiles/walls/bottom_right_wall.png"),
		textures_path("tiles/walls/bottom_right_wall_normal.png"),
		textures_path("tiles/walls/bottom_right_wall_corner2.png"),
		textures_path("tiles/walls/bottom_right_wall_corner2_normal.png"),
		textures_path("tiles/walls/bottom_left_wall_corner2.png"),
		textures_path("tiles/walls/bottom_left_wall_corner2_normal.png"),
		textures_path("tiles/walls/top_right_wall_corner2.png"),
		textures_path("tiles/walls/top_right_wall_corner2_normal.png"),
		textures_path("tiles/walls/top_left_wall_corner2.png"),
		textures_path("tiles/walls/top_left_wall_corner2_normal.png"),
		textures_path("tiles/walls/bottom_right_wall_corner.png"),
		textures_path("tiles/walls/bottom_right_wall_corner_normal.png"),
		textures_path("tiles/walls/bottom_left_wall_corner.png"),
		textures_path("tiles/walls/bottom_left_wall_corner_normal.png"),
		textures_path("tiles/walls/top_right_wall_corner.png"),
		textures_path("tiles/walls/top_right_wall_corner_normal.png"),
		textures_path("tiles/walls/top_left_wall_corner.png"),
		textures_path("tiles/walls/top_left_wall_corner_normal.png"),
		textures_path("tiles/floors/floor_1.png"),
		textures_path("tiles/floors/floor_1_normal.png"),
		textures_path("tiles/floors/floor_2.png"),
		textures_path("tiles/floors/floor_2_normal.png"),
		textures_path("tiles/floors/floor_3.png"),
		textures_path("tiles/floors/floor_4.png"),
		textures_path("tiles/floors/floor_5.png"),
		textures_path("tiles/floors/hole.png"),
		textures_path("effects/laser.png"),
		textures_path("enemies/enemy_basic.png"),
		textures_path("enemies/dead_enemy.png"),
		textures_path("ui/crosshair.png"),
		textures_path("ui/health_border.png"),
		textures_path("ui/health_00.png"),
		textures_path("ui/health_01.png"),
		textures_path("ui/health_02.png"),
		textures_path("ui/health_03.png"),
		textures_path("ui/health_04.png"),
		textures_path("ui/health_05.png"),
		textures_path("ui/health_06.png"),
		textures_path("ui/title_screen.png"),
		textures_path("cinematic/cutscene_0.png"),
		textures_path("cinematic/cutscene_1.png"),
		textures_path("cinematic/cutscene_2.png"),
		textures_path("cinematic/cutscene_3.png"),
		textures_path("cinematic/cutscene_4.png"),
		textures_path("cinematic/cutscene_5.png"),
		textures_path("cinematic/cutscene_6.png"),
		textures_path("cinematic/cutscene_7.png"),
		textures_path("cinematic/cutscene_8.png"),
		textures_path("ui/start_text.png"),
		textures_path("ui/resume_text.png"),
		textures_path("ui/options_text.png"),
		textures_path("ui/quit_text.png"),
		textures_path("ui/outline_for_testing_button.png"),
		textures_path("ui/stamina_00.png"),
		textures_path("ui/stamina_01.png"),
		textures_path("ui/stamina_02.png"),
		textures_path("ui/stamina_03.png"),
		textures_path("ui/stamina_04.png"),
		textures_path("ui/stamina_05.png"),
		textures_path("ui/stamina_06.png"),
		textures_path("ui/stamina_07.png"),
		textures_path("ui/stamina_08.png"),
		textures_path("ui/stamina_09.png"),
		textures_path("ui/stamina_10.png"),
		textures_path("ui/stamina_11.png"),
		textures_path("ui/stamina_12.png"),
		textures_path("ui/stamina_13.png"),
		textures_path("ui/stamina_14.png"),
		textures_path("ui/stamina_15.png"),
		textures_path("ui/stamina_16.png"),
		textures_path("effects/muzzle_flash0.png"),
		textures_path("effects/muzzle_flash1.png"),
		textures_path("effects/muzzle_flash2.png"),
		textures_path("effects/wall_particle_0.png"),
		textures_path("effects/wall_particle_1.png"),
		textures_path("effects/wall_particle_2.png"),
		textures_path("effects/wall_particle_3.png"),
		textures_path("effects/wall_particle_4.png"),
		textures_path("effects/blood_splatter_0.png"),
		textures_path("effects/blood_splatter_1.png"),
		textures_path("effects/blood_splatter_2.png"),
		textures_path("effects/blood_splatter_3.png"),
		textures_path("effects/blood_splatter_4.png"),
		textures_path("effects/blood_splatter_5.png"),
		textures_path("effects/blood_splatter_6.png"),
		textures_path("effects/blood_splatter_7.png"),
		textures_path("effects/damage_indicator_0.png"),
		textures_path("effects/damage_indicator_1.png"),
		textures_path("effects/damage_indicator_2.png"),
		textures_path("effects/damage_indicator_3.png"),
		textures_path("instructions/instruction_WASD.png"),
		textures_path("instructions/instruction_left_click.png"),
		textures_path("instructions/instruction_space.png"),
		textures_path("instructions/instruction_R.png"),
		textures_path("instructions/instruction_health_pack.png"),
		textures_path("instructions/instruction_dodge.png"),
		textures_path("instructions/instruction_melee.png"),
		textures_path("props/container.png"),
		textures_path("props/container_disabled.png"),
		textures_path("props/server_disabled.png"),
		textures_path("props/server.png"),
		textures_path("props/server_down.png"),
		textures_path("props/server_normal.png"),
		textures_path("props/chest_base.png"),
		textures_path("props/chest_normal.png"),
		textures_path("props/chest_open.png"),
		textures_path("props/chest_open_normal.png"),
		textures_path("props/chest0.png"),
		textures_path("props/chest0_disabled.png"),
		textures_path("props/chest0_enabled.png"),
		textures_path("lights/RED_LIGHT.png"),
		textures_path("lights/RED_LIGHT_END.png"),
		textures_path("lights/BLUE_LIGHT.png"),
		textures_path("lights/BLUE_LIGHT_END.png"),
		textures_path("pickups/health_box.png"),
		textures_path("pickups/pistol_pickup.png"),
		textures_path("pickups/smg_pickup.png"),
		textures_path("pickups/shotgun_pickup.png"),
		textures_path("pickups/railgun_pickup.png"),
		textures_path("pickups/revolver_pickup.png"),
		textures_path("ui/pistol_ui.png"),
		textures_path("projectiles/slash_0.png"),
		textures_path("projectiles/slash_1.png"),
		textures_path("projectiles/slash_2.png"),
		textures_path("projectiles/slash_3.png"),
		textures_path("ui/shotgun_ui.png"),
		textures_path("ui/smg_ui.png"),
		textures_path("ui/level_0_text.png"),
		textures_path("ui/level_1_text.png"),
		textures_path("ui/level_2_text.png"),
		textures_path("ui/level_3_text.png"),
		textures_path("ui/level_4_text.png"),
		textures_path("ui/the_end_text.png"),
		textures_path("doors/door_closed_blue.png"),
		textures_path("doors/door_closed_blue_normal.png"),
		textures_path("doors/left_door.png"),
		textures_path("doors/right_door.png"),
		textures_path("doors/top_door.png"),
		textures_path("doors/top_left_door_normal.png"),
		textures_path("doors/top_right_door_normal.png"),
		textures_path("doors/top_door_normal.png"),
		textures_path("effects/door_particle_0.png"),
		textures_path("effects/door_particle_1.png"),
		textures_path("effects/door_particle_2.png"),
		textures_path("effects/door_particle_3.png"),
		textures_path("effects/door_particle_4.png"),
		textures_path("effects/door_particle_5.png"),
		textures_path("effects/door_particle_6.png"),
		textures_path("effects/door_particle_7.png"),
		textures_path("effects/door_particle_8.png"),
		textures_path("effects/door_particle_9.png"),
		textures_path("ui/railgun_ui.png"),
		textures_path("ui/revolver_ui.png"),

	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("textured"),
		shader_path("screen_effect"),
		shader_path("lights"),
		shader_path("multiply"),
		shader_path("shadows"),
		shader_path("textured_normal"),
		shader_path("font")
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);
	
	void initializeCamera();

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	void initializeFonts();

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the vignette shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	mat3 create_inverse_projection_matrix(Motion& camera_motion);

	mat3 create_inverse_projection_matrix();

	mat3 create_ortho_matrix(int width, int height, bool y_down = true, bool top_left_origin = true);

	Entity get_screen_state_entity() { return screen_state_entity; }

	vec2 wcs_to_dcs(vec2 wcs_position);

	vec2 dcs_to_wcs(Motion& camera_motion, vec2 dcs);

	Entity create_tile_render_request(Tile tile, int col, int row);

	void set_map_texture(Map& map);

	void make_quad(std::vector<vec3>& vertices, float x1, float x2, float y1, float y2);

	bool is_light_in_view(const Light& light, const mat3& projection_matrix);

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawTexturedNormal(Entity entity, const mat3& projection);
	void renderText(Text& text, Motion& motion, mat3 projection);
	void drawToScreen();
	void drawAllLights();
	void drawLight(mat3 projection_matrix, mat3 inverse_projection, Light& light);
	void drawShadowVolume(mat3 projection_matrix, Light& light);
	void drawUI(mat3 projection);
	void drawTextureIgnoreLighting(mat3 projection);

	// Window handle
	GLFWwindow* window;

	GLuint frame_buffer;

	// Screen texture handles
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;
	GLuint base_texture;
	GLuint base_texture_normal;
	GLuint light_texture;
	GLuint shadow_texture;

	GLuint vao;

	Entity screen_state_entity;
	Entity camera_entity;

	struct {
		GLint light_pos;
		GLint light_radius;
		GLint light_intensity;
		GLint light_color;
		GLint light_is_local;
		GLint light_height;
		GLint u_time;
		GLint inverse_projection;
		GLint normal_map;
		GLint shadow_map;
		GLint in_position;
	} light_uniforms;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
