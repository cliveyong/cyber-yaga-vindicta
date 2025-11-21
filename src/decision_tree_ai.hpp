#ifndef DECISION_TREE_HPP
#define DECISION_TREE_HPP

#include <functional>
#include <memory>

#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "common.hpp"            
#include "map_init.hpp"         

// Base class for decision tree nodes.
class decision_node {
public:
    virtual ~decision_node() {}
    // Evaluate this node for a given enemy; returns an action.
    virtual ENEMY_ACTION evaluate(Entity enemy) = 0;
};

// decision node that holds a condition, branches to one of two 
// child nodes based on whether the condition is true
class condition_node : public decision_node {
public:
    std::function<bool(Entity)> condition;
    std::unique_ptr<decision_node> true_branch;
    std::unique_ptr<decision_node> false_branch;

    condition_node(
        std::function<bool(Entity)> cond,
        std::unique_ptr<decision_node> t_branch,
        std::unique_ptr<decision_node> f_branch
    );

    virtual ENEMY_ACTION evaluate(Entity enemy) override;
};


// leaf node that returns a fixed action.
class action_node : public decision_node {
public:
    ENEMY_ACTION action;
    action_node(ENEMY_ACTION act);

    virtual ENEMY_ACTION evaluate(Entity enemy) override;
};

// Helper functions used by the decision tree.

// Returns the Euclidean distance between the enemy and the player.
float get_distance_to_player(Entity enemy);

// Returns true if the enemy is within a specified range of the player.
bool is_within_range(Entity enemy, float range);

// Returns true if the enemy and the player are in the same room.
// uses the grid coordinates (based on GRID_CELL_SIZE) and the map's room data.
bool is_enemy_in_same_room_as_player(Entity enemy);

bool enemy_has_los_to_player(Entity enemy);

bool has_path_validated(Entity enemy);

 //Function to build the decision tree.
// updated for milestone 3, handling for pathfinding

std::unique_ptr<decision_node> build_decision_tree();

#endif