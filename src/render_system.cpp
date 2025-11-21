
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <set>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

void RenderSystem::drawTexturedMesh(Entity entity, const mat3 &projection)
{
	//Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;

	if (registry.motions.has(entity)) {
		Motion& motion = registry.motions.get(entity);
		transform.translate(motion.position);
		transform.scale(motion.scale);
		transform.rotate(radians(motion.angle));
	}
	else if (registry.buttons.has(entity)) {
		UIButton& btn = registry.buttons.get(entity);
		transform.translate(btn.position);
		transform.scale(btn.scale);
	}
	else if (registry.text.has(entity)) {
		TitleScreenText& text = registry.text.get(entity);
		transform.translate(text.position);
		transform.scale(text.scale);
	}


	/*transform.translate(motion.position);
	transform.scale(motion.scale);
	transform.rotate(radians(motion.angle));*/

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)EFFECT_ASSET_ID::TEXTURED;
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// texture-mapped entities - use data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
						  sizeof(TexturedVertex), (void *)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3)); // note the stride to skip the preceeding vertex position

	GLint albedo_loc = glGetUniformLocation(program, "albedo");
	glUniform1i(albedo_loc, 0);
	gl_has_errors();

	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	if (render_request.texture_origin == TEXTURE_ORIGIN_ID::FILE) { // TODO: See if we can refactor to remove if and assert
		GLuint texture_id = texture_gl_handles[(GLuint)render_request.used_texture];
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}
	else if (registry.renderRequests.get(entity).texture_origin == TEXTURE_ORIGIN_ID::MAP) {
		int current_level = registry.gameProgress.components[0].level;
		Map& map = registry.maps.components[current_level];
		glBindTexture(GL_TEXTURE_2D, map.map_texture);
	}
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	GLint alpha_uloc = glGetUniformLocation(program, "alpha");
	glUniform1f(alpha_uloc, registry.renderRequests.get(entity).alpha);
	gl_has_errors();
	
	GLint outline_loc = glGetUniformLocation(program, "outline_transparency");
	GLint outline_color_a_loc = glGetUniformLocation(program, "outline_color_a");
	GLint outline_color_b_loc = glGetUniformLocation(program, "outline_color_b");
	glUniform1f(outline_loc, render_request.outline_transparency);
	glUniform3f(outline_color_a_loc, render_request.outline_color_a.x, render_request.outline_color_a.y, render_request.outline_color_a.z);
	glUniform3f(outline_color_b_loc, render_request.outline_color_b.x, render_request.outline_color_b.y, render_request.outline_color_b.z);

	GLint time_loc = glGetUniformLocation(program, "time");
	glUniform1f(time_loc, glfwGetTime());

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawTexturedNormal(Entity entity, const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale);
	transform.rotate(radians(motion.angle));

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)EFFECT_ASSET_ID::TEXTURED_WITH_NORMAL;
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
						  sizeof(TexturedVertex), (void *)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3)); // note the stride to skip the preceeding vertex position

	GLint albedo_loc = glGetUniformLocation(program, "albedo");
	GLint normal_loc = glGetUniformLocation(program, "normal");

	glUniform1i(albedo_loc, 0);
	glUniform1i(normal_loc, 1);
	gl_has_errors();

	gl_has_errors();

	assert(registry.renderRequests.has(entity));
	if (registry.renderRequests.get(entity).texture_origin == TEXTURE_ORIGIN_ID::FILE) { // TODO: See if we can refactor to remove if and assert
		glActiveTexture(GL_TEXTURE0);
		GLuint texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glActiveTexture(GL_TEXTURE1);
		texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).texture_normal];
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}
	else if (registry.renderRequests.get(entity).texture_origin == TEXTURE_ORIGIN_ID::MAP) {
		glActiveTexture(GL_TEXTURE0);
		int current_level = registry.gameProgress.components[0].level;
		Map& map = registry.maps.components[current_level];
		glBindTexture(GL_TEXTURE_2D, map.map_texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, map.map_normal);
	}
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::renderText(Text& text, Motion& motion, mat3 projection_matrix) {

    GLuint text_program = effects[(GLuint)EFFECT_ASSET_ID::FONT];
    glUseProgram(text_program);
    const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::TEXT];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Grab shader uniforms
    auto projectionLoc = glGetUniformLocation(text_program, "projection");
    GLint textColor_location = glGetUniformLocation(text_program, "textColor");

    // Set the projection matrix and text color
    glUniformMatrix3fv(projectionLoc, 1, GL_FALSE, (float*)&projection_matrix);
    glUniform3f(textColor_location, text.color.x, text.color.y, text.color.z);

    // Batch characters by texture
    std::unordered_map<GLuint, std::vector<float>> batchedVertices;
    float offset = 0.f;
    for (char c : text.content) {
        Character character = registry.character_map[c];

        float xpos = motion.position.x + offset + character.Bearing.x;
        float ypos = motion.position.y - character.Bearing.y;
        float w = character.Size.x * motion.scale.x;
        float h = character.Size.y * motion.scale.y;

        batchedVertices[character.TextureID].insert(
            batchedVertices[character.TextureID].end(), {
                xpos,     ypos + h,  0.0f, 1.0f,  // Top-left
                xpos,     ypos,      0.0f, 0.0f,  // Bottom-left
                xpos + w, ypos,      1.0f, 0.0f,  // Bottom-right

                xpos,     ypos + h,  0.0f, 1.0f,  // Top-left
                xpos + w, ypos,      1.0f, 0.0f,  // Bottom-right
                xpos + w, ypos + h,  1.0f, 1.0f   // Top-right
            }
        );

        offset += (character.Advance >> 6) * motion.scale.x;
    }

    // Render each batch
    for (const auto& [textureID, vertices] : batchedVertices) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4);
    }
}

// first draw to an intermediate texture,
// apply the "vignette" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the vignette texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE]);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	gl_has_errors();

	// add the "vignette" effect
	const GLuint vignette_program = effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE];

	// set clock
	GLuint time_uloc       = glGetUniformLocation(vignette_program, "time");
	GLuint glitch_start_uloc = glGetUniformLocation(vignette_program, "glitch_remaining");
	GLuint glitch_duration_uloc = glGetUniformLocation(vignette_program, "glitch_duration");

	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	
	ScreenState& screen = registry.screen_state;
	glUniform1f(glitch_start_uloc, screen.glitch_remaining_ms);
	glUniform1f(glitch_duration_uloc, screen.glitch_duration);
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(vignette_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	// Draw
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr); 
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, base_texture, 0);
	
	// clear backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	
	// white background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();

	mat3 projection_2D = createProjectionMatrix();

	if (debugging.wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Draw renderRequests in order of their z_index
	// This runs in linear time in relation to the number of renderRequests
	// Sorting would have been O(n*log(n))
	for (int z_index=0; z_index < (int)Z_INDEX::NO_LIGHTING; z_index++) {
		for (int i=0; i < registry.renderRequests.size(); i++) {
			Entity entity = registry.renderRequests.entities[i];
			RenderRequest& renderRequest = registry.renderRequests.components[i];
			if (z_index == (int) renderRequest.z_index && registry.motions.has(entity) && !renderRequest.is_ui_element) {
				drawTexturedMesh(entity, projection_2D);
			}
		}
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, base_texture_normal, 0);

	for (int z_index=0; z_index < (int)Z_INDEX::NO_LIGHTING; z_index++) {
		for (int i=0; i < registry.renderRequests.size(); i++) {
			Entity entity = registry.renderRequests.entities[i];
			RenderRequest& renderRequest = registry.renderRequests.components[i];
			if (z_index == (int) renderRequest.z_index && registry.motions.has(entity) && !renderRequest.is_ui_element) {
				drawTexturedNormal(entity, projection_2D);
			}
		}
	}

	if (debugging.wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	drawAllLights();

	// draw framebuffer to screen
	// adding "vignette" effect when applied
	drawToScreen();

	drawTextureIgnoreLighting(projection_2D);
	drawUI(ortho_matrix);

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// M1: creative element #21: Camera control 
	// Uses camera component resolution and position to create the projection matrix, rather than having it fixed at the window size and center.
	Motion& motion = registry.motions.get(camera_entity);

	float x_pos = motion.position.x;
	float y_pos = motion.position.y;
	float height = motion.scale.y;
	float width = motion.scale.x;
	
	float left   = x_pos - width/2;
	float top    = y_pos - height/2;
	float right  = x_pos + width/2;
	float bottom = y_pos + height/2;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
		{ sx, 0.f, 0.f},
		{0.f,  sy, 0.f},
		{ tx,  ty, 1.f}
	};
}

mat3 RenderSystem::create_inverse_projection_matrix(Motion& camera_motion) {
	float x_pos = camera_motion.position.x;
	float y_pos = camera_motion.position.y;
	float height = camera_motion.scale.y;
	float width = camera_motion.scale.x;
	
	float left   = x_pos - width/2;
	float top    = y_pos - height/2;
	float right  = x_pos + width/2;
	float bottom = y_pos + height/2;

	float sx = (right - left) / 2.f;
    float sy = (top - bottom) / 2.f;
    float tx = (right + left) / 2.f;
    float ty = (top + bottom) / 2.f;

    return {
        { sx, 0.f, 0.f },
        { 0.f, sy, 0.f },
        { tx, ty, 1.f }
    };
}

mat3 RenderSystem::create_inverse_projection_matrix() {
	Motion& camera_motion = registry.motions.get(camera_entity);

	float x_pos = camera_motion.position.x;
	float y_pos = camera_motion.position.y;
	float height = camera_motion.scale.y;
	float width = camera_motion.scale.x;
	
	float left   = x_pos - width/2;
	float top    = y_pos - height/2;
	float right  = x_pos + width/2;
	float bottom = y_pos + height/2;

	float sx = (right - left) / 2.f;
    float sy = (top - bottom) / 2.f;
    float tx = (right + left) / 2.f;
    float ty = (top + bottom) / 2.f;

    return {
        { sx, 0.f, 0.f },
        { 0.f, sy, 0.f },
        { tx, ty, 1.f }
    };
}


// Converts World Coordinate System (WCS) positions to Device Coordinate System (DCS) positions
vec2 RenderSystem::wcs_to_dcs(vec2 wcs) {
	mat3 projection = createProjectionMatrix();
	vec3 ndcs = projection * vec3(wcs, 1.0f);
	vec2 dcs = {
		(ndcs.x * 0.5f + 0.5f) * WINDOW_WIDTH_PX,				// From [-1,1] to [0, width]
		(1.0f - (ndcs.y * 0.5f + 0.5f)) * WINDOW_HEIGHT_PX		// From [-1,1] to [0, height], flip Y
	};
	return dcs;
}

vec2 RenderSystem::dcs_to_wcs(Motion& camera_motion, vec2 dcs) {
	mat3 projection = create_inverse_projection_matrix(camera_motion);
	float scale_x_reciprocal = 1.f / camera_motion.scale.x;
	float scale_y_reciprocal = 1.f / camera_motion.scale.y;
	vec2 normalized_dcs = {
		(2.f * dcs.x) * scale_x_reciprocal - 1.f,
		1.f - (2.f * dcs.y) * scale_y_reciprocal
	};
	vec3 world_coords = projection * vec3(normalized_dcs.x, normalized_dcs.y, 1.f);
	return { world_coords.x, world_coords.y };
}

Entity RenderSystem::create_tile_render_request(Tile tile, int col, int row) {
	Entity entity = Entity();

	Mesh& mesh = getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = grid_to_world_coord(col, row);
	motion.scale = vec2({ GRID_CELL_SIZE, GRID_CELL_SIZE });

	registry.renderRequests.insert(
		entity,
		{
			tile.texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			Z_INDEX::FLOOR,
			false,
			TEXTURE_ORIGIN_ID::FILE,
			tile.normal
		}
	);
	return entity;
}

mat3 RenderSystem::create_ortho_matrix(int width, int height, bool y_down, bool top_left_origin) {
	float left = 0.0f;
    float right = width;
    float bottom = height;
    float top = 0.0f;

    if (!top_left_origin) {
        left = -width / 2.0f;
        right = width / 2.0f;
        bottom = -height / 2.0f;
        top = height / 2.0f;
    }

    if (y_down) {
		float temp = top;
		top = bottom;
		bottom = temp;
    }

    float sx = 2.0f / (right - left);
    float sy = 2.0f / (top - bottom);
    float tx = - (right + left) / (right - left);
    float ty = - (top + bottom) / (top - bottom);

    return {
        { sx,  0.f,  0.f },
        { 0.f, sy,   0.f },
        { tx,  ty,   1.f }
    };
}

void RenderSystem::set_map_texture(Map& map) {
	if (map.map_texture != 0) {
		glDeleteTextures(1, &map.map_texture);
	}

	if (map.map_normal != 0) {
		glDeleteTextures(1, &map.map_normal);
	}

	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &map.map_texture);
	glBindTexture(GL_TEXTURE_2D, map.map_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map.grid_width * GRID_CELL_SIZE, map.grid_height * GRID_CELL_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &map.map_normal);
	glBindTexture(GL_TEXTURE_2D, map.map_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map.grid_width * GRID_CELL_SIZE, map.grid_height * GRID_CELL_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, map.map_texture, 0);
	glViewport(0, 0, map.grid_width * GRID_CELL_SIZE, map.grid_height * GRID_CELL_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	std::vector<Entity> temp_entities;
	std::set<int> visited;
	for (int row = 0; row < map.grid_height; row++) {
		for (int col = 0; col < map.grid_width; col++) {
			int id = map.tile_object_grid[row][col];
			if (visited.find(id) != visited.end()) {
				continue;
			}
			visited.insert(id);

			Tile tile = map.tiles[id];
			if (tile.id == TILE_ID::EMPTY) {
				continue;
			}

			Entity entity = create_tile_render_request(tile, col, row);
			temp_entities.push_back(entity);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mat3 projection_2D = create_ortho_matrix(map.grid_width * GRID_CELL_SIZE, map.grid_height * GRID_CELL_SIZE, true, true);
	for (int z_index=0; z_index < z_index_count; z_index++) {
		for (int i=0; i < temp_entities.size(); i++) {
			Entity entity = temp_entities[i];
			RenderRequest& renderRequest = registry.renderRequests.components[i];
			if (z_index == (int) renderRequest.z_index) {
				drawTexturedMesh(entity, projection_2D);
			}
		}
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, map.map_normal, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int z_index=0; z_index < z_index_count; z_index++) {
		for (int i=0; i < temp_entities.size(); i++) {
			Entity entity = temp_entities[i];
			RenderRequest& renderRequest = registry.renderRequests.components[i];
			if (z_index == (int) renderRequest.z_index) {
				drawTexturedNormal(entity, projection_2D);
			}
		}
	}

	glDisable(GL_BLEND);

	glDeleteFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	for (Entity entity : temp_entities) {
		registry.remove_all_components_of(entity);
	}

	std::vector<vec3> vertices;
	for (ShadowCaster& shadowCaster : registry.shadowCasters.components) {
		make_quad(vertices, shadowCaster.start.x, shadowCaster.start.y, shadowCaster.end.x, shadowCaster.end.y);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SHADOW_QUAD]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
}

void RenderSystem::make_quad(std::vector<vec3>& vertices, float x1, float y1, float x2, float y2) { // Consider building these in the GPU using a normalized unit quad and a transform matrix
	vertices.push_back({ x1, y1, 0.f }); // Top-right
	vertices.push_back({ x1, y1, 1.f }); // Top left
	vertices.push_back({ x2, y2, 0.f }); // Bottom right
	vertices.push_back({ x1, y1, 1.f }); // Top-left
	vertices.push_back({ x2, y2, 0.f }); // Bottom-right
	vertices.push_back({ x2, y2, 1.f }); // Bottom-left
}

void RenderSystem::drawAllLights() {
	mat3 projection_matrix = createProjectionMatrix();
	mat3 inverse_projection = create_inverse_projection_matrix();
	ScreenState& screen_state = registry.screen_state;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_texture, 0);

	glClearColor(screen_state.ambient_color.x, screen_state.ambient_color.y, screen_state.ambient_color.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (Light& light : registry.lights.components) {
		if (!is_light_in_view(light, projection_matrix)) {
			continue;
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow_texture, 0);
		drawShadowVolume(projection_matrix, light);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_texture, 0);
		drawLight(projection_matrix, inverse_projection, light);
		glDisable(GL_BLEND);
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, off_screen_render_buffer_color, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, base_texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, light_texture);

	const GLuint multiply_program = effects[(GLuint)EFFECT_ASSET_ID::MULTIPLY];
	glUseProgram(multiply_program);

	GLuint map_texture_loc = glGetUniformLocation(multiply_program, "map_texture");
	GLuint light_texture_loc = glGetUniformLocation(multiply_program, "light_texture");
	GLint in_position_loc = glGetAttribLocation(multiply_program, "in_position");
	
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);

	glUniform1i(map_texture_loc, 0);
	glUniform1i(light_texture_loc, 1);

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr); 
}

void RenderSystem::drawShadowVolume(mat3 projection_matrix, Light& light) {
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLuint shadow_program = effects[(GLuint)EFFECT_ASSET_ID::SHADOWS];
	glUseProgram(shadow_program);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SHADOW_QUAD]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	GLuint light_pos_loc = glGetUniformLocation(shadow_program, "light_pos");
	GLuint projection_loc = glGetUniformLocation(shadow_program, "projection");
	glUniform2f(light_pos_loc, light.position.x, light.position.y);
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection_matrix);

	GLint size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_vertices = size / sizeof(vec3);

	glDrawArrays(GL_TRIANGLES, 0, num_vertices);
}

void RenderSystem::drawLight(mat3 projection_matrix, mat3 inverse_projection, Light& light) {
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);

	const GLuint light_program = effects[(GLuint)EFFECT_ASSET_ID::LIGHT];
	glUseProgram(light_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, base_texture_normal);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadow_texture);

	glUniform2f(light_uniforms.light_pos, light.position.x, light.position.y);
    glUniform1f(light_uniforms.light_radius, light.radius);
    glUniform1f(light_uniforms.light_height, light.height);
    glUniform1f(light_uniforms.light_intensity, light.intensity);
    glUniform1f(light_uniforms.light_is_local, light.is_local);
    glUniform3f(light_uniforms.light_color, light.color.x, light.color.y, light.color.z);
	glUniform1f(light_uniforms.u_time, glfwGetTime());
	glUniformMatrix3fv(light_uniforms.inverse_projection, 1, GL_FALSE, (float*)&inverse_projection);

	glEnableVertexAttribArray(light_uniforms.in_position);
	glVertexAttribPointer(light_uniforms.in_position, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr); 
}

void RenderSystem::drawUI(mat3 projection) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int z_index = (int)Z_INDEX::NO_LIGHTING; z_index < z_index_count; z_index++) {
		for (Entity& entity : registry.uis.entities) {
			if (registry.texts.has(entity)) {
				if (z_index == (int)Z_INDEX::HUD) {
					Motion& motion = registry.motions.get(entity);
					Text& text = registry.texts.get(entity);
					renderText(text, motion, projection);
				}
			} else {
				RenderRequest& render_request = registry.renderRequests.get(entity);
				if ((int)render_request.z_index == z_index) {
					drawTexturedMesh(entity, projection);
				}
			}
		}
	}

	glDisable(GL_BLEND);
}

void RenderSystem::drawTextureIgnoreLighting(mat3 projection) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (Entity& entity : registry.textureWithoutLighting.entities) {
		RenderRequest& render_request = registry.renderRequests.get(entity);
		drawTexturedMesh(entity, projection);
	}
	glDisable(GL_BLEND);
}

bool RenderSystem::is_light_in_view(const Light& light, const mat3& projection_matrix) {
	vec2 light_center_ndc = projection_matrix * vec3(light.position.x, light.position.y, 1.0);
    
    float scale_x = std::abs(projection_matrix[0][0]);
    float scale_y = std::abs(projection_matrix[1][1]);
    
    float radius_ndc = light.radius * std::max(scale_x, scale_y);
    
    float buffer = 0.1f;
    radius_ndc *= (1.0f + buffer);
    
    if (light_center_ndc.x + radius_ndc < -1.0 || 
        light_center_ndc.x - radius_ndc > 1.0 ||
        light_center_ndc.y + radius_ndc < -1.0 || 
        light_center_ndc.y - radius_ndc > 1.0) {
        return false;
    }
    
    return true;
}