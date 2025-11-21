#include "decision_tree_ai.hpp"

#include <cmath>
#include <iostream>
#include "ai_system.hpp"
#include "tinyECS/registry.hpp"
#include "a_star_pathfinding.hpp"
#include <glm/trigonometric.hpp>
// (Ensure this header gives access to registry.enemies, registry.players, etc.)

// holds a condition and branches to a child node based on whether condition returns true or false
condition_node::condition_node(std::function<bool(Entity)> cond,
    std::unique_ptr<decision_node> tBranch,
    std::unique_ptr<decision_node> fBranch)
    : condition(cond), true_branch(std::move(tBranch)), false_branch(std::move(fBranch))
{
}

// evaluates the resulting action
ENEMY_ACTION condition_node::evaluate(Entity enemy) {
    if (condition(enemy))
        return true_branch->evaluate(enemy);
    else
        return false_branch->evaluate(enemy);
}

// leaf node. returns an action (do_nothing, combat, move etc.)
action_node::action_node(ENEMY_ACTION act)
    : action(act)
{
}

ENEMY_ACTION action_node::evaluate(Entity enemy) {
    return action;
}

// Helper function
float get_distance_to_player(Entity enemy) {
    if (registry.players.entities.empty()) {
        return 1e6f; // return a very large value
    } 
    Entity player = registry.players.entities[0];
    auto& enemy_motion = registry.motions.get(enemy);
    auto& player_motion = registry.motions.get(player);
    return length(player_motion.position - enemy_motion.position);
}

// helper function
bool is_within_range(Entity enemy, float range) {
    return get_distance_to_player(enemy) <= range;
}

// helper function
bool is_enemy_in_same_room_as_player(Entity enemy) {
    if (registry.players.entities.empty() || registry.maps.components.empty())
        return false;

    Entity player = registry.players.entities[0];
    auto& enemyMotion = registry.motions.get(enemy);
    auto& playerMotion = registry.motions.get(player);
    int current_level = registry.gameProgress.components[0].level;
    Map& map = registry.maps.components[current_level];

    ivec2 player_grid_pos = world_to_grid_coords(playerMotion.position.x, playerMotion.position.y);
    ivec2 enemy_grid_pos = world_to_grid_coords(enemyMotion.position.x, enemyMotion.position.y);

    return (map.room_mask[player_grid_pos.y][player_grid_pos.x] == 
        map.room_mask[enemy_grid_pos.y][enemy_grid_pos.x]);
}

// Check for line-of-sight using a_star_pathfinding function.
bool enemy_has_los_to_player(Entity enemy) {
    if (registry.players.entities.empty() || registry.maps.components.empty())
        return false;
    Entity player = registry.players.entities[0];
    auto& enemy_motion = registry.motions.get(enemy);
    auto& player_motion = registry.motions.get(player);
    // Call the global has_line_of_sight from a_star_pathfinding.cpp.
    int current_level = registry.gameProgress.components[0].level;
    Map& map = registry.maps.components[current_level];
    return has_line_of_sight(enemy_motion.position, player_motion.position, map);
};

// returns true if enemy facing player
bool is_facing_player(Entity enemy) {
    if (registry.players.entities.empty())
        return false;

    Entity player = registry.players.entities[0];
    auto& enemy_motion = registry.motions.get(enemy);
    auto& player_motion = registry.motions.get(player);

    // vector from the enemy to the player
    vec2 to_player = player_motion.position - enemy_motion.position;
    float distance = length(to_player);
    
    // if at same location
    if (distance == 0.f)
        return true;

    to_player = to_player / distance;

    // compute enemy's forward vector from its angle
    // note: angle is assumed to be in degrees
    float enemy_angle_rad = glm::radians(enemy_motion.angle);
    vec2 enemy_forward = { cos(enemy_angle_rad), sin(enemy_angle_rad) };

    // get cosine of the angle between enemy_forward and toPlayer
    float dot_val = glm::dot(enemy_forward, to_player);

    float cone_half_angle = 80.f; // degrees
    float threshold = cos(glm::radians(cone_half_angle));

    return (dot_val >= threshold);
}

// returns true if the the enemy has reached its destination ie. last known player location
bool has_path_reached(Entity enemy) {
    if (!registry.pathComponents.has(enemy))
        return false;
    auto& pc = registry.pathComponents.get(enemy);
    bool ret_val = pc.current_index >= pc.waypoints.size();
    //std::cout << "has path reached: " << ret_val << std::endl;
    return ret_val;
}

bool has_path_validated(Entity enemy) {
    return registry.pathComponents.get(enemy).valid == true;
}

std::unique_ptr<decision_node> build_decision_tree() {
    // decisions if enemy is idle
    auto idle_branch = std::make_unique<condition_node>(
        /*
        * check if los and facing player
        * if true, then
        *     check if backoff distance
        *     if true, action backoff
        *     if false then
        *         check if combat distance
        *         if true, action combat
        *         if false then
        *             check if pursuit distance
        *             if true, action pursuit
        *             if false action idle
        * if false, action idle
        */

        [](Entity enemy) -> bool {
            return (is_facing_player(enemy) && enemy_has_los_to_player(enemy));
        },
        std::make_unique<condition_node>(
            [](Entity enemy) -> bool {
                return is_within_range(enemy, registry.enemies.get(enemy).backoff_range);
            },
            std::make_unique<action_node>(ENEMY_ACTION::ACTION_BACKOFF),
            std::make_unique<condition_node>(
                [](Entity enemy) -> bool {
                    return is_within_range(enemy, registry.enemies.get(enemy).attack_range);
                },
                std::make_unique<action_node>(ENEMY_ACTION::ACTION_COMBAT),
                std::make_unique<condition_node>(
                    [](Entity enemy) -> bool {
                        return is_within_range(enemy, registry.enemies.get(enemy).pursuit_range);
                    },
                    std::make_unique<action_node>(ENEMY_ACTION::ACTION_PURSUIT),
                    std::make_unique<action_node>(ENEMY_ACTION::ACTION_IDLE)
                )
            )
        ),
        std::make_unique<action_node>(ENEMY_ACTION::ACTION_IDLE)
    );
    //decisions if enemy is in pursuit
    auto pursuit_branch = std::make_unique<condition_node>(
        /*
        * check if path reached
        * if true then
        *     check if facing player and los
        *     if true, action pursuit
        *     if false then
        *         check if path is valid
        *         if true, action idle
        *         if false, action pursuit
        * if false then
        *     check if LOS and facing player
        *     if true, then
        *         check if backoff distance
        *         if true, action backoff
        *         if false then
        *             check if combat distance
        *             if true, action combat
        *             if false then
        *                 check pursuit distance
        *                 if true, action pursuit
        *                 if false, action idle
        *     if false, action pursuit
        */   

        [](Entity enemy) -> bool {
            return (has_path_reached(enemy));
        },
        std::make_unique<condition_node>(
            [](Entity enemy) -> bool {
                return (is_facing_player(enemy) && enemy_has_los_to_player(enemy)); 
            },
            std::make_unique<action_node>(ENEMY_ACTION::ACTION_PURSUIT),
            std::make_unique<condition_node>(
                [](Entity enemy) -> bool {
                    return has_path_validated(enemy);
                },
                std::make_unique<action_node>(ENEMY_ACTION::ACTION_IDLE),
                std::make_unique<action_node>(ENEMY_ACTION::ACTION_PURSUIT)
            )
        ),
        std::make_unique<condition_node>(
            [](Entity enemy) -> bool {
                return (is_facing_player(enemy) && enemy_has_los_to_player(enemy));
            },
            std::make_unique<condition_node>(
                [](Entity enemy) -> bool {
                    return is_within_range(enemy, registry.enemies.get(enemy).backoff_range);
                },
                std::make_unique<action_node>(ENEMY_ACTION::ACTION_BACKOFF),
                std::make_unique<condition_node>(
                    [](Entity enemy) -> bool {
                        return is_within_range(enemy, registry.enemies.get(enemy).attack_range);
                    },
                    std::make_unique<action_node>(ENEMY_ACTION::ACTION_COMBAT),
                    std::make_unique<condition_node>(
                        [](Entity enemy) -> bool {
                            return is_within_range(enemy, registry.enemies.get(enemy).pursuit_range);
                        },
                        std::make_unique<action_node>(ENEMY_ACTION::ACTION_PURSUIT),
                        std::make_unique<action_node>(ENEMY_ACTION::ACTION_IDLE)
                    )
                )
            ),
            std::make_unique<action_node>(ENEMY_ACTION::ACTION_PURSUIT)
        )
    );
    // decisions if enemy is in combat
    auto combat_branch = std::make_unique<condition_node>(
        /*
        * check if los and facing player
        * if true then
        *     check if backoff distance
        *     if true, action backoff
        *     if false then
        *         check if combat distance
        *         if true, action combat
        *         if false, action pursuit
        * if false, action pursuit
        */

        [](Entity enemy) -> bool {
            return (is_facing_player(enemy) && enemy_has_los_to_player(enemy));
        },
        std::make_unique<condition_node>(
            [](Entity enemy) -> bool {
                return is_within_range(enemy, registry.enemies.get(enemy).backoff_range);
            },
            std::make_unique<action_node>(ENEMY_ACTION::ACTION_BACKOFF),
            std::make_unique<condition_node>(
                [](Entity enemy) -> bool {
                    return is_within_range(enemy, registry.enemies.get(enemy).attack_range);
                },
                std::make_unique<action_node>(ENEMY_ACTION::ACTION_COMBAT),
                std::make_unique<action_node>(ENEMY_ACTION::ACTION_PURSUIT)
            )
        ),
        std::make_unique<action_node>(ENEMY_ACTION::ACTION_PURSUIT)
    );
    // decisions if enemy is in backoff
    auto backoff_branch = std::make_unique<condition_node>(
        /*
        * check if backoff distance
        * if true, action backoff
        * if false then
        *     check if los and facing player and combat distance
        *     if true, action combat
        *     if false, action pursuit
        */

        [](Entity enemy) -> bool {
            return is_within_range(enemy, registry.enemies.get(enemy).backoff_range);
        },
        std::make_unique<action_node>(ENEMY_ACTION::ACTION_BACKOFF),
        std::make_unique<condition_node>(
            [](Entity enemy) -> bool {
                return (is_facing_player(enemy) && 
                    is_within_range(enemy, registry.enemies.get(enemy).backoff_range) && 
                    enemy_has_los_to_player(enemy));
            },
            std::make_unique<action_node>(ENEMY_ACTION::ACTION_COMBAT),
            std::make_unique<action_node>(ENEMY_ACTION::ACTION_IDLE)
        )
    );

    // root tree. does these checks and from them branches to each sub tree
    auto root = std::make_unique<condition_node>(
        [](Entity enemy) -> bool {
            return (registry.enemies.get(enemy).state == ENEMY_STATE::IDLE); // check if enemy state is IDLE
        },
        std::move(idle_branch), // if true then move to idle branch
        std::make_unique<condition_node>( // if false then
            [](Entity enemy) -> bool {
                return (registry.enemies.get(enemy).state == ENEMY_STATE::PURSUIT); // check if enemy state is pursuit
            },
            std::move(pursuit_branch), // if true then move to pursuit branch
            std::make_unique<condition_node>( // if false then
                [](Entity enemy) -> bool {
                    return (registry.enemies.get(enemy).state == ENEMY_STATE::COMBAT); // check if enemy state is COMBAT
                },
                std::move(combat_branch), // if true then move to combat branch
                std::move(backoff_branch) // if false then must be in backoff, move to backoff branch
            )
        )
    );
    return root;
};