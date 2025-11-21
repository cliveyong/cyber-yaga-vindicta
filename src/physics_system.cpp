// internal
#include "physics_system.hpp"
#include "physics_system_init.hpp"
#include "world_init.hpp"
#include <iostream>
#include <array>
#include <glm/trigonometric.hpp>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// Returns the relative center of the collision object based on angle
vec2 get_relative_center(vec2 position, float angle, vec2 offset) {
	float radians = glm::radians(angle);
	vec2 rotated_offset = {
		offset.x * cos(radians) - offset.y * sin(radians),
		offset.x * sin(radians) + offset.y * cos(radians)
	};
	return position + rotated_offset;
}

// Collision detection using circle-sweep check
bool CircleBoundCollides(Motion& motion_i, vec2& position_i, CircleBound& circle_bound_i, Motion& motion_j, vec2& position_j, CircleBound& circle_bound_j, const float delta_time)
{
	// calculate the distance between the centers of both entities
	float radius_i = circle_bound_i.collision_radius;
	float radius_j = circle_bound_j.collision_radius;
	float radius_sum = radius_i + radius_j;
	float radius_sum_squared = radius_sum * radius_sum;

	vec2 velocity_i = motion_i.velocity * delta_time;
	vec2 velocity_j = motion_j.velocity * delta_time;

	// relative motion components
	vec2 relative_position = position_i - position_j;
	vec2 relative_velocity = velocity_i - velocity_j;

	// quadratic function variables from the collision formula: ((pi(t) - pj(t))^2 = (ri + rj)^2 
	// where:
	// p(t) = initial position + velocity * t 
	// relative_position = pi - pj
	// relative velocity = vi - vj
	float a = glm::dot(relative_velocity, relative_velocity);
	float b = 2.0f * glm::dot(relative_position, relative_velocity);
	float c = glm::dot(relative_position, relative_position) - radius_sum_squared;

	// discriminant = b^2 -4ac
	float discriminant = b * b - 4.0f * a * c;

	if (c <= 0.0f) {
		return true;
	}

	if (discriminant < 0.0f) {
		return false;
	}

	float discriminant_sqrt = sqrt(discriminant);
	float time_1 = (-b - discriminant_sqrt) / (2.0f * a);
	float time_2 = (-b + discriminant_sqrt) / (2.0f * a);

	// Check if the collision occurs within the time step (0 ≤ t ≤ 1)\
	// time_1 = 0 means they collide at the start of the frame
	// 0 < time_1 < 1 means they collide during this frame
	// time_1 > 1 means they collide in a future frame
	if (time_1 >= 0.0f && time_1 <= 1.0f) {
		vec2 collision = motion_i.position + velocity_i * time_1;
		//std::cout << "PHYSICS SYSTEM: COLLISION DETECTED AT: (" << collision.x << " , " << collision.y << ")" << std::endl;
		return true;
	}

	//std::cout << "PHYSICS SYSTEM: NO COLLISION DETECTED" << std::endl;
	return false;
}

// Rotate a point around the origin
vec2 rotate_point(const vec2& point, float sin_angle, float cos_angle) {
	return vec2{
		point.x * cos_angle - point.y * sin_angle,
		point.x * sin_angle + point.y * cos_angle
	};
}

// Project a point onto an axis using dot product
float project_point_on_axis(const vec2& point, const vec2& axis) {
	return point.x * axis.x + point.y * axis.y;
}

// Calculate the corners of an AABB after rotation
std::array<vec2, 4> get_rotated_corners(const vec2& center, const vec2& size, float angle) {
	// Half width and half height
	float x_half = size.x / 2.0f;
	float y_half = size.y / 2.0f;

	// AABB corners before rotation
	std::array<vec2, 4> local_corners = {{
		{ -x_half, -y_half }, // Top-left
		{ x_half, -y_half },  // Top-right
		{ x_half, y_half },   // Bottom-right
		{ -x_half, y_half }   // Bottom-left
	}};

	float radians_angle = radians(angle);
	float sin_angle = sin(radians_angle);
	float cos_angle = cos(radians_angle);

	// Rotate each corner and apply the center position
	std::array<vec2, 4> corners;
    for (int i = 0; i < 4; ++i) {
        corners[i] = rotate_point(local_corners[i], sin_angle, cos_angle);
        corners[i].x += center.x;
        corners[i].y += center.y;
    }

    return corners;
}

float squared_distance(const vec2& a, const vec2& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// Check if two AABBs overlap using SAT with rotation
bool AABBSAT(vec2& center_i, std::array<vec2, 4>& corners_i, AABB& AABB_i, vec2& center_j, std::array<vec2, 4>& corners_j, AABB& AABB_j, float delta_time) {
	// Add early exit check
    float half_width_i = AABB_i.collision_box.x / 2.0f;
    float half_height_i = AABB_i.collision_box.y / 2.0f;
    float radius_i = std::sqrt(half_width_i * half_width_i + half_height_i * half_height_i);

    float half_width_j = AABB_j.collision_box.x / 2.0f;
    float half_height_j = AABB_j.collision_box.y / 2.0f;
    float radius_j = std::sqrt(half_width_j * half_width_j + half_height_j * half_height_j);

    float center_distance_squared = squared_distance(center_i, center_j);
    float total_radius = radius_i + radius_j;
    if (center_distance_squared > total_radius * total_radius) {
        return false;
    }


	// Check for overlap along the axes defined by the edges of the AABBs
	vec2 axes[4];

	// AABBs axes (edges of both AABBs)
	for (int i = 0; i < 4; ++i) {
		vec2 edge_i = vec2{ corners_i[(i + 1) % 4].x - corners_i[i].x, corners_i[(i + 1) % 4].y - corners_i[i].y };
		vec2 edge_j = vec2{ corners_j[(i + 1) % 4].x - corners_j[i].x, corners_j[(i + 1) % 4].y - corners_j[i].y };

		// Calculate the normal to the edge (perpendicular axis)
		axes[i] = vec2{ -edge_i.y, edge_i.x };  // Normal of AABB_i's edge
		if (i >= 2) axes[i] = vec2{ -edge_j.y, edge_j.x };  // Normal of AABB_j's edge
	}

	// For each axis, project the corners of both AABBs
	for (int i = 0; i < 4; ++i) {
		// Project the corners of entity_i onto the axis
		float min_i = project_point_on_axis(corners_i[0], axes[i]);
		float max_i = min_i;
		for (int j = 1; j < 4; ++j) {
			// project points onto axis
			float projection = project_point_on_axis(corners_i[j], axes[i]);
			min_i = std::min(min_i, projection);
			max_i = std::max(max_i, projection);
		}

		// Project the corners of entity_j onto the axis
		float min_j = project_point_on_axis(corners_j[0], axes[i]);
		float max_j = min_j;
		for (int j = 1; j < 4; ++j) {
			float projection = project_point_on_axis(corners_j[j], axes[i]);
			min_j = std::min(min_j, projection);
			max_j = std::max(max_j, projection);
		}

		// Check for overlap along this axis
		if (max_i < min_j || max_j < min_i) {
			return false;
		}
	}
	// Overlap on all axes, collision detected
	return true;
}


bool AABBCircleSAT(vec2& rect_center, AABB& rect_aabb, std::array<vec2, 4>& rect_corners, vec2& circle_center, CircleBound& circle_bound, Motion& circle_motion, float delta_time) {
	// Early exit check
	float max_rect_radius = std::max(rect_aabb.collision_box.x, rect_aabb.collision_box.y) / 2.0f;
    float max_distance = max_rect_radius + circle_bound.collision_radius;
	float max_distance_squared = max_distance * max_distance;
	float center_distance_x = rect_center.x - circle_center.x;
	float center_distance_y = rect_center.y - circle_center.y;
	float center_distance_squared = center_distance_x * center_distance_x + center_distance_y * center_distance_y;
    if (center_distance_squared > max_distance_squared) {
        return false;
    }

	// Generate axes from rectangle edges
	vec2 axes[3]; // 2 rectangle edge normals + 1 extra from circle to closest point
	for (int i = 0; i < 2; ++i) {
		vec2 edge = rect_corners[(i + 1) % 4] - rect_corners[i];
		axes[i] = vec2(-edge.y, edge.x); // Perpendicular edge normal
	}

	// Find the closest corner on the rectangle to the circle's center
	vec2 closest_point = rect_corners[0];
	float min_dist_sq = std::numeric_limits<float>::max();
	for (int i = 0; i < 4; ++i) {
		vec2 corner = rect_corners[i];
		// squared distance between corner to circle center
		float dist_sq = (corner.x - circle_center.x) * (corner.x - circle_center.x) +
			(corner.y - circle_center.y) * (corner.y - circle_center.y);
		if (dist_sq < min_dist_sq) {
			min_dist_sq = dist_sq;
			closest_point = corner;
		}
	}

	// axis from circle's center to closest corner on AABB
	axes[2] = closest_point - circle_center;

	// TODO: Look into working with squared distances until we have to normalize, do this for the other collision functions too
	// SAT Projection Tests
	for (int i = 0; i < 3; ++i) {
		vec2 axis = axes[i];
		float axis_length_sq = axis.x * axis.x + axis.y * axis.y;
		if (axis_length_sq == 0) {
			continue;
		}
		// normalize axis
		axis = axis / sqrt(axis_length_sq);

		// Project rectangle onto axis
		float min_rect = project_point_on_axis(rect_corners[0], axis);
		float max_rect = min_rect;
		for (int j = 1; j < 4; ++j) {
			float proj = project_point_on_axis(rect_corners[j], axis);
			min_rect = std::min(min_rect, proj);
			max_rect = std::max(max_rect, proj);
		}

		// Project circle onto axis
		float proj_circle = project_point_on_axis(circle_center, axis);
		float min_circle = proj_circle - circle_bound.collision_radius;
		float max_circle = proj_circle + circle_bound.collision_radius;

		// TODO: Don't we also have to check if the rectangle is moving and it will pass the circle?
		// check if the circle will fully pass the rectangle in the next frame
		vec2 circle_next_position = circle_center + circle_motion.velocity * delta_time;
		float proj_circle_next = project_point_on_axis(circle_next_position, axis);
		float min_circle_next = proj_circle_next - circle_bound.collision_radius;
		float max_circle_next = proj_circle_next + circle_bound.collision_radius;
		if ((max_rect < min_circle_next || max_circle_next < min_rect) &&
			(max_rect < min_circle || max_circle < min_rect)) {
			//std::cout << "Circle will pass the rectangle in the next frame!" << std::endl;
			return false;
		}

		// If there's a gap between projections, no collision
		if (max_rect < min_circle || max_circle < min_rect) {
			return false;
		}
	}
	// Overlap on all axes, collision detected
	return true;
}

// given the vertices, updates from local to world coordinate
std::vector<vec2> update_world_vertices(const Motion& motion, const meshCollidable& mesh) {
	std::vector<vec2> world_vertices;
	float cosA = cos(motion.angle);
	float sinA = sin(motion.angle);

	for (const auto& v : mesh.vertices) {

		// Apply rotation using 2D rotation matrix
		vec2 rotated = {
			v.x * cosA - v.y * sinA,
			v.x * sinA + v.y * cosA
		};

		// Apply translation (move to world position)
		vec2 worldPos = rotated + motion.position;

		world_vertices.push_back(worldPos);
	}

	return world_vertices;
}


void project_mesh_onto_axis(const std::vector<vec2>& vertices, const vec2& axis, float& min, float& max) {
	min = max = glm::dot(vertices[0], axis);
	for (const auto& v : vertices) {
		float projection = glm::dot(v, axis);
		if (projection < min) min = projection;
		if (projection > max) max = projection;
	}
}

void project_aabb_onto_axis(const std::array<vec2, 4>& corners, const vec2& axis, float& min, float& max) {
	min = max = glm::dot(corners[0], axis);
	for (const auto& v : corners) {
		float projection = glm::dot(v, axis);
		if (projection < min) min = projection;
		if (projection > max) max = projection;
	}
}

// collision check between mesh and AABB bounding box that has rotated using SAT
bool MeshAABBSATCollision(const meshCollidable& mesh, const Motion& mesh_motion, const AABB& aabb, const Motion& aabb_motion) {
	const std::vector<vec2>& world_vertices = update_world_vertices(mesh_motion, mesh);
	vec2 rect_center = get_relative_center(aabb_motion.position, aabb_motion.angle, aabb.offset);
	const std::array<vec2, 4>& aabb_corners = get_rotated_corners(rect_center, aabb.collision_box, aabb_motion.angle);

	// Calculate the axes of the mesh
	std::vector<vec2> axes;
	for (size_t i = 0; i < world_vertices.size(); i++) {
		size_t next = (i + 1) % world_vertices.size();
		vec2 edge = world_vertices[next] - world_vertices[i];
		vec2 normal = { -edge.y, edge.x };
		axes.push_back(glm::normalize(normal));
	}

	// Calculate the axes of the rotated AABB
	for (size_t i = 0; i < aabb_corners.size(); i++) {
		size_t next = (i + 1) % aabb_corners.size();
		vec2 edge = aabb_corners[next] - aabb_corners[i];
		vec2 normal = { -edge.y, edge.x };
		axes.push_back(glm::normalize(normal));
	}

	// Project the mesh and AABB onto each axis
	for (const auto& axis : axes) {
		float minMesh, maxMesh;
		float minAABB, maxAABB;

		// Project mesh vertices onto axis
		project_mesh_onto_axis(world_vertices, axis, minMesh, maxMesh);

		// Project AABB corners onto axis
		project_aabb_onto_axis(aabb_corners, axis, minAABB, maxAABB);

		// If projections do not overlap, no collision
		if (maxMesh < minAABB || maxAABB < minMesh) {
			return false;
		}
	}
	// If we didn't find a separating axis, there is a collision
	return true;
}

// Helper function to project the circle onto an axis
void project_circle_onto_axis(const vec2& center, float radius, const vec2& axis, float& min, float& max) {
	float projection = glm::dot(center, axis);
	float offset = radius * glm::length(axis);
	min = projection - offset;
	max = projection + offset;
}

// Collision check between mesh and circle using SAT
bool MeshCircleSATCollision(const meshCollidable& mesh, const Motion& mesh_motion, const vec2& circle_center, float circle_radius, const Motion& circle_motion) {
	// Get the world vertices of the mesh
	const std::vector<vec2>& world_vertices = update_world_vertices(mesh_motion, mesh);
	// Calculate the axes of the mesh
	std::vector<vec2> axes;
	for (size_t i = 0; i < world_vertices.size(); i++) {
		size_t next = (i + 1) % world_vertices.size();
		vec2 edge = world_vertices[next] - world_vertices[i];
		vec2 normal = glm::normalize(vec2(-edge.y, edge.x));
		axes.push_back(normal);
	}

	// Project the mesh and circle onto each axis
	for (const auto& axis : axes) {
		float minMesh, maxMesh;
		float minCircle, maxCircle;

		// Project mesh vertices onto axis`
		project_mesh_onto_axis(world_vertices, axis, minMesh, maxMesh);

		// Project circle onto axis
		project_circle_onto_axis(circle_center, circle_radius, axis, minCircle, maxCircle);

		// If projections do not overlap, no collision
		if (maxMesh < minCircle || maxCircle < minMesh) {
			return false;
		}
	}

	// If we didn't find a separating axis, there is a collision
	return true;
}


void PhysicsSystem::step(float elapsed_ms)
{
	float delta_time = elapsed_ms / 1000.f;

	// Move each entity that has motion (invaders, projectiles, and even towers [they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	auto& projectiles_registry = registry.projectiles;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];

		motion.position += motion.velocity * delta_time;
		motion.angle = (int)(motion.angle + motion.angle_velocity) % 360;
		if (motion.angle < 0) motion.angle += 360;
	}

	for (uint i = 0; i < projectiles_registry.size(); i++) {
		Entity entity = projectiles_registry.entities[i];
		Projectile& projectile = projectiles_registry.components[i];

		if (projectile.can_bounce || projectile.is_gun) {
			Motion& motion = motion_registry.get(entity);
			float drag = 0.1f;      // Lower drag value = faster slowdown
			float ang_drag = 0.1f;  // Lower angular drag = faster spin decay

			// exponential drag for rotation
			motion.angle_velocity *= pow(ang_drag, delta_time);

			// exponential drag for movement
			motion.velocity *= pow(drag, delta_time);

			float angular_velocity_threshold = 0.5f;

			if (std::abs(motion.angle_velocity) < angular_velocity_threshold && std::abs(motion.angle_velocity) > 0.f) {
				// TODO: Refactor this
				motion.angle_velocity = 0.0f;

				if (projectile.is_gun) {
					if (!registry.pickups.has(entity)) {
						Gun& gun = registry.guns.get(entity);
						RenderRequest& render = registry.renderRequests.get(entity);
						render.z_index = Z_INDEX::PICKUP;
						render.used_texture = gun.thrown_sprite;
						if (gun.gun_type == GUN_TYPE::SMG) {
							motion.scale = vec2(GRID_CELL_SIZE, GRID_CELL_SIZE);
						}
						else if (gun.gun_type == GUN_TYPE::SHOTGUN) {
							motion.scale = vec2(GRID_CELL_SIZE * 1.5, GRID_CELL_SIZE * 1.5);
						}
						registry.textureWithoutLighting.emplace(entity);
						Pickup& pickup = registry.pickups.emplace(entity);
						pickup.gun_type = gun.gun_type;
						pickup.type = PICKUP_TYPE::GUN;
						pickup.value = gun.current_magazine + gun.remaining_bullets;
						pickup.range = GRID_CELL_SIZE * 1.5;
					}
				}
			}
			
			// Stop when almost still
			if (length(motion.velocity) < 100.0f && std::abs(motion.angle_velocity) > angular_velocity_threshold) {
				motion.velocity = { 0.0f, 0.0f };
				Gun& gun = registry.guns.get(entity);
				registry.renderRequests.remove(entity);
				Entity pickup_entity = spawn_pickup(motion.position, motion.angle, PICKUP_TYPE::GUN, gun.current_magazine + gun.remaining_bullets, gun.gun_type);
				registry.guns.insert(pickup_entity, gun);
				registry.projectiles.remove(entity);

			}

			if (motion.velocity.x <= 4.f && motion.velocity.y <= 4.f && !projectile.is_gun) {
				registry.remove_all_components_of(entity);
			}
		}
	}

	ComponentContainer<MovingCollidable>& moving_collidables = registry.movingCollidables;
	ComponentContainer<StaticCollidable>& static_collidables = registry.staticCollidables;
	ComponentContainer<MovingCircle>& moving_circle_collidables = registry.movingCircleCollidables;
	ComponentContainer<MovingSAT>& moving_SAT_collidables = registry.movingSATCollidables;
	ComponentContainer<meshCollidable>& mesh_collidables = registry.meshCollidables;
	ComponentContainer<Pickup>& pickups = registry.pickups;
	Entity player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);

	SpatialHash& spatial_hash = registry.spatialHashes.components[0];

	for (uint i = 0; i < moving_circle_collidables.components.size(); i++) {
		Entity entity_i = moving_circle_collidables.entities[i];
		Motion& motion_i = registry.motions.get(entity_i);
		CircleBound& circle_bound_i = registry.circlebounds.get(entity_i);
		vec2 circle_center_i = get_relative_center(motion_i.position, motion_i.angle, circle_bound_i.offset);
		std::vector<Entity> potential_static_collisions = get_potential_collisions(spatial_hash, entity_i, motion_i);

		for (Entity entity_j : potential_static_collisions) {
			Motion& motion_j = registry.motions.get(entity_j);
			AABB& aabb_j = registry.AABBs.get(entity_j);
			vec2 rect_center_j = get_relative_center(motion_j.position, motion_j.angle, aabb_j.offset);
			std::array<vec2, 4> rect_corners_j = get_rotated_corners(rect_center_j, aabb_j.collision_box, motion_j.angle);

			if (AABBCircleSAT(rect_center_j, aabb_j, rect_corners_j, circle_center_i, circle_bound_i, motion_i, delta_time)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
			}
		}

		// Commented out because right now we don't need any circle-circle collisions
		//for (uint j = i + 1; j < moving_circle_collidables.components.size(); j++) {
		//	Entity entity_j = moving_circle_collidables.entities[j];
		//	Motion& motion_j = registry.motions.get(entity_j);
		//	CircleBound& circle_bound_j = registry.circlebounds.get(entity_j);
		//	vec2 circle_center_j = get_relative_center(motion_j.position, motion_j.angle, circle_bound_j.offset);
		//	if (CircleBoundCollides(motion_i, circle_center_i, circle_bound_i, motion_j, circle_center_j, circle_bound_j, delta_time)) {
		//		registry.collisions.emplace_with_duplicates(entity_i, entity_j);
		//	}
		//}

		for (uint j = 0; j < moving_SAT_collidables.components.size(); j++) {
			Entity entity_j = moving_SAT_collidables.entities[j];
			Motion& motion_j = registry.motions.get(entity_j);
			AABB& aabb_j = registry.AABBs.get(entity_j);
			vec2 rect_center_j = get_relative_center(motion_j.position, motion_j.angle, aabb_j.offset);
			std::array<vec2, 4> rect_corners_j = get_rotated_corners(rect_center_j, aabb_j.collision_box, motion_i.angle);
			if (AABBCircleSAT(rect_center_j, aabb_j, rect_corners_j, circle_center_i, circle_bound_i, motion_i, delta_time)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
			}
		}
	}

	for (uint i = 0; i < moving_SAT_collidables.components.size(); i++) {
		Entity entity_i = moving_SAT_collidables.entities[i];
		Motion& motion_i = registry.motions.get(entity_i);
		AABB& aabb_i = registry.AABBs.get(entity_i);
		vec2 rect_center_i = get_relative_center(motion_i.position, motion_i.angle, aabb_i.offset);
		std::array<vec2, 4> rect_corners_i = get_rotated_corners(rect_center_i, aabb_i.collision_box, motion_i.angle);
		std::vector<Entity> potential_static_collisions = get_potential_collisions(spatial_hash, entity_i, motion_i);

		for (Entity entity_j : potential_static_collisions) {
			Motion& motion_j = registry.motions.get(entity_j);
			AABB& aabb_j = registry.AABBs.get(entity_j);
			vec2 rect_center_j = get_relative_center(motion_j.position, motion_j.angle, aabb_j.offset);
			std::array<vec2, 4> rect_corners_j = get_rotated_corners(rect_center_j, aabb_j.collision_box, motion_j.angle);

			if (AABBSAT(rect_center_j, rect_corners_j, aabb_j, rect_center_i, rect_corners_i, aabb_i, delta_time)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
			}
		}
	}


	for (uint i = 0; i < mesh_collidables.components.size(); i++) {
		Entity entity_i = mesh_collidables.entities[i];
		Motion& motion_i = registry.motions.get(entity_i);
		meshCollidable& mesh_i = registry.meshCollidables.get(entity_i);

		for (uint j = i + 1; j < moving_circle_collidables.components.size(); j++) {
			Entity entity_j = moving_circle_collidables.entities[j];
			Motion& motion_j = registry.motions.get(entity_j);
			CircleBound& circle_bound_j = registry.circlebounds.get(entity_j);
			vec2 circle_center_j = get_relative_center(motion_j.position, motion_j.angle, circle_bound_j.offset);
			if (MeshCircleSATCollision(mesh_i, motion_i, circle_center_j, circle_bound_j.collision_radius, motion_j)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
			}
		}

		for (uint j = 0; j < moving_SAT_collidables.components.size(); j++) {
			Entity entity_j = moving_SAT_collidables.entities[j];
			Motion& motion_j = registry.motions.get(entity_j);
			AABB& aabb_j = registry.AABBs.get(entity_j);
			if (MeshAABBSATCollision(mesh_i, motion_i, aabb_j, motion_j)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
			}
		}
	}

	// Pickup collisions
	for (uint i = 0; i < pickups.components.size(); i++) {
		Pickup pickup = pickups.components[i];
		Entity pickup_entity = pickups.entities[i];
		Motion& pickup_motion = registry.motions.get(pickup_entity);
		float dist_to_player = length(pickup_motion.position - player_motion.position);
		if (dist_to_player < pickup.range) {
			registry.collisions.emplace_with_duplicates(player_entity, pickup_entity);
		}
	}
}