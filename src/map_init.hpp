#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include <unordered_map>

Map create_map_struct(int x, int y);

Entity create_map(int x, int y);

Room create_room(int x, int y, bool generate_walls);

Tile get_random_floor_tile();

void generate_walls_around_room(Room& room);

void add_tile_to_room(Room& room, ivec2 pos, Tile tile);

void add_prop_to_room(Room& room, Prop prop, vec2 pos);

void add_prop_to_map(Map& map, Prop prop, vec2 pos);

void add_door_prop(Map& map, Prop prop, vec2 pos);

void fill_missing_with_floor(Room& room);

bool add_room_to_map(Map& map, Room& room, ivec2 pos);

bool add_door(Map& map, ivec2 pos);

void update_door_tiles(Map& map, ivec2 pos, int size, bool vertical);

void add_light(Map& map, vec2 pos, vec3 color, float radius, float intensity, float height);

void add_temp_light(vec2 pos, vec3 color, float radius, float intensity, bool is_local, float timer);

void add_pickup_to_map(Map& map, vec2 grid_pos, PICKUP_TYPE pickup_type, int value, GUN_TYPE gun_type);

void add_enemy_to_map(Map& map, ivec2 grid_position, GUN_TYPE gun_type, float health, float speed_factor, float detection_range_factor, float attack_range_factor);

void update_tile_grid(Map& map);

void set_tile(Map& map, ivec2 pos, Tile tile);

TILE_ID get_tile(Map& map, ivec2 pos);

WALL_DIRECTION get_wall_direction(TEXTURE_ASSET_ID texture);

Map load_map_from_file(const std::string& name);

std::pair<std::string, int> global_to_local_id(int gid, const std::vector<TilesetInfo>& tilesets);

const Tile get_tile_from_tileset(const std::string& tileset_name, int lid);

const Prop get_prop_from_tileset(const std::string& tileset_name, int lid);

void create_map_0();

void create_map_1();

void create_map_2();

void create_map_3();