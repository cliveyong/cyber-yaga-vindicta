#include "a_star_pathfinding.hpp"
#include <limits>
#include <algorithm>

// each grid cell is a unique key
static inline int cell_key(const ivec2& cell, int grid_width) {
    return cell.y * grid_width + cell.x;
}

// use the "came from" map to reconstruct the path
static std::vector<ivec2> reconstruct_path(const std::unordered_map<int, ivec2>& came_from, const ivec2& current, int grid_width) {
    std::vector<ivec2> total_path;
    ivec2 curr = current;
    total_path.push_back(curr);
    int key = cell_key(curr, grid_width);
    while (came_from.find(key) != came_from.end()) {
        curr = came_from.at(key);
        total_path.push_back(curr);
        key = cell_key(curr, grid_width);
    }
    std::reverse(total_path.begin(), total_path.end());
    return total_path;
}

std::vector<ivec2> find_path(const ivec2& start, const ivec2& goal, const Map& map) {
        std::priority_queue<Node, std::vector<Node>, NodeComparator> open_set;
    std::unordered_map<int, float> g_score;
    std::unordered_map<int, ivec2> came_from;
    int gridWidth = map.grid_width;

    auto cell_key = [&](const ivec2& c) {
        return c.y * gridWidth + c.x;
    };

    int startKey = cell_key(start);
    float start_h = heuristic(start, goal);
    g_score[startKey] = 0.f;
    open_set.push(Node(start, 0.f, start_h, start));

    // 8 directions: up/down/left/right + diagonals
    std::vector<std::pair<ivec2, float>> directions = {
        { {0, -1}, 1.f },
        { {0,  1}, 1.f },
        { {-1, 0}, 1.f },
        { { 1, 0}, 1.f },

        // diagonal movements. comment out if using manhattan
        { { 1,  1}, 1.4142f },
        { { 1, -1}, 1.4142f },
        { {-1,  1}, 1.4142f },
        { {-1, -1}, 1.4142f }
    };

    while (!open_set.empty()) {
        Node current = open_set.top();
        open_set.pop();

        // if goal is reached
        if (current.pos == goal) {
            return reconstruct_path(came_from, current.pos, gridWidth);
        }

        for (auto& dirPair : directions) {
            ivec2 delta = dirPair.first;
            ivec2 neighbor = {
                current.pos.x + dirPair.first.x,
                current.pos.y + dirPair.first.y
            };

            // this check ensures that path does not go diagonal if a wall collision would end up stopping the path from being reached
            if (abs(delta.x) == 1 && abs(delta.y) == 1) {
                ivec2 neighbor_horizontal = { current.pos.x + delta.x, current.pos.y };
                ivec2 neighbor_vertical = { current.pos.x, current.pos.y + delta.y };
                if (!traversable(neighbor_horizontal, map) || !traversable(neighbor_vertical, map)) {
                    continue; // skip diagonal neighbor if one of the adjacent cells is blocked
                }
            }

            // in general skip blocked or out-of-bounds
            if (!traversable(neighbor, map)) 
                continue;

            float tentative_g = current.g + dirPair.second;
            int neighKey = cell_key(neighbor);
            if (g_score.find(neighKey) == g_score.end() || tentative_g < g_score[neighKey]) {
                came_from[neighKey] = current.pos;
                g_score[neighKey] = tentative_g;
                float h = heuristic(neighbor, goal);
                open_set.push(Node(neighbor, tentative_g, h, current.pos));
            }
        }
    }
    // no path found
    return {};
}

bool has_line_of_sight(const vec2& start, const vec2& end, const Map& map) {
    ivec2 start_cell = world_to_grid_coords(start.x, start.y);
    ivec2 end_cell = world_to_grid_coords(end.x, end.y);

    int x0 = start_cell.x, y0 = start_cell.y;
    int x1 = end_cell.x, y1 = end_cell.y;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        // if we find a collidable block ie. walls
        if (!traversable({ x0, y0 }, map)) {
            return false;
        }
        // go until the end
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy; x0 += sx;
        }
        if (e2 <= dx) {
            err += dx; y0 += sy;
        }
    }
    return true;
}

