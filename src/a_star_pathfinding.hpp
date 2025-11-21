#pragma once
#include "common.hpp"
#include "map_init.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <iostream>

// A* node structure
struct Node {
    ivec2 pos;
    float g;   // cost from start
    float h;   // heuristic
    float f;   // total cost
    ivec2 parent;

    Node(ivec2 pos_, float g_, float h_, ivec2 parent_)
        : pos(pos_), g(g_), h(h_), f(g_ + h_), parent(parent_) {
    }
};

// comparator for the priority queue
struct NodeComparator {
    bool operator()(const Node& a, const Node& b) const {
        return a.f > b.f;
    }
};

// Using octile
inline float heuristic(const ivec2& a, const ivec2& b) {
    // Euclidean distance
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);
    return std::sqrt(dx * dx + dy * dy);
}

// Checks the map cell and returns true if the cell is traversable (non-wall)
inline bool traversable(const ivec2& cell, const Map& map) {
    if (cell.x < 0 || cell.y < 0 || cell.x >= map.grid_width || cell.y >= map.grid_height)
        return false;
    return (map.tile_id_grid[cell.y][cell.x] != TILE_ID::WALL && map.tile_id_grid[cell.y][cell.x] != TILE_ID::CLOSED_DOOR);
}

std::vector<ivec2> find_path(const ivec2& start, const ivec2& goal, const Map& map);

bool has_line_of_sight(const vec2& start, const vec2& end, const Map& map);
