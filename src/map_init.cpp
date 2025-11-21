#include "map_init.hpp"
#include "props.hpp"
#include "tiles.hpp"
#include "tinyECS/registry.hpp"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h" 

#include <vector>
#include <fstream>
#include <iostream>
#include <map>

Map create_map_struct(int grid_width, int grid_height) {
    Map map;

    map.grid_height = grid_height;
    map.grid_width = grid_width;

    map.tile_id_grid.resize(grid_height, std::vector<TILE_ID>(grid_width, TILE_ID::EMPTY));
    map.tile_object_grid.resize(grid_height, std::vector<int>(grid_width, -1));
    map.room_mask.resize(grid_height, std::vector<int>(grid_width, -1));
    map.wall_directions.resize(grid_height, std::vector<WALL_DIRECTION>(grid_width, WALL_DIRECTION::NONE));

    return map;
}

Entity create_map(int grid_width, int grid_height) {
    Entity ent = Entity();    
    registry.maps.insert(ent, create_map_struct(grid_width, grid_height));
    return ent;
}

Room create_room(int room_width, int room_height, bool generate_walls) {
    Room room = Room();
    room.room_height = room_height;
    room.room_width = room_width;
	std::unordered_map<int, Tile> room_tiles;
	std::vector<std::vector<int>> room_grid;
    room.room_grid.resize(room_height, std::vector<int>(room_width, -1));
    room.wall_directions.resize(room_height, std::vector<WALL_DIRECTION>(room_width, WALL_DIRECTION::NONE));
    
    if (generate_walls) {
        generate_walls_around_room(room);
    }

    return room;
}

void generate_walls_around_room(Room& room) {
	for (int row = 1; row < room.room_height - 1; row++) {
        add_tile_to_room(room, { 0, row }, LEFT_WALL1);
        add_tile_to_room(room, { room.room_width - 1, row }, RIGHT_WALL1);
	}

	for (int col = 1; col < room.room_width - 1; col++) {
        add_tile_to_room(room, { col, 0 }, TOP_WALL1);
        add_tile_to_room(room, { col, room.room_height - 1 }, BOTTOM_WALL1);
	}

    add_tile_to_room(room, { 0, 0 }, TOP_LEFT_WALL1);
    add_tile_to_room(room, { 0, room.room_height - 1 }, BOTTOM_LEFT_WALL1);
    add_tile_to_room(room, { room.room_width - 1, 0 }, TOP_RIGHT_WALL1);
    add_tile_to_room(room, { room.room_width - 1, room.room_height - 1 }, BOTTOM_RIGHT_WALL1);
}

Tile get_random_floor_tile() {
    int random_i = rand() % FLOOR_TILES.size();
    return FLOOR_TILES[random_i];
}

void add_tile_to_room(Room& room, ivec2 pos, Tile tile) {
    int x = pos.x;
    int y = pos.y;
    int width = tile.size.x;
    int height = tile.size.y;
    if (x + width - 1 >= room.room_width || x < 0 || y + height - 1 >= room.room_height || y < 0) {
        return;
    }

	WALL_DIRECTION direction = get_wall_direction(tile.texture);
    int id = (int)Entity();
    room.room_tiles.emplace(id, tile);
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            room.room_grid[i][j] = id;
            room.wall_directions[y][x] = direction;
        }
    }
}

void add_prop_to_room(Room& room, Prop prop, vec2 pos) {
    room.props[pos] = prop;
}

// adds a destructible prop door. If door is vertical, use the left-most and bottom-most tile
void add_door_prop(Map& map, Prop prop, vec2 pos) {
    map.props[pos] = prop;
    map.prop_doors_list.push_back(pos);
}

void add_prop_to_map(Map& map, Prop prop, vec2 pos) {
    map.props[pos] = prop;
}

WALL_DIRECTION get_wall_direction(TEXTURE_ASSET_ID texture) {
    if (texture == TEXTURE_ASSET_ID::TOP_WALL1) { return WALL_DIRECTION::TOP; }
    if (texture == TEXTURE_ASSET_ID::RIGHT_WALL1) { return WALL_DIRECTION::RIGHT; };
    if (texture == TEXTURE_ASSET_ID::BOTTOM_WALL1) { return WALL_DIRECTION::BOTTOM; }
    if (texture == TEXTURE_ASSET_ID::LEFT_WALL1) { return WALL_DIRECTION::LEFT; }
    if (texture == TEXTURE_ASSET_ID::TOP_LEFT_WALL1) { return WALL_DIRECTION::TOP_LEFT; }
    if (texture == TEXTURE_ASSET_ID::TOP_RIGHT_WALL1) { return WALL_DIRECTION::TOP_RIGHT; }
    if (texture == TEXTURE_ASSET_ID::BOTTOM_LEFT_WALL1) { return WALL_DIRECTION::BOTTOM_LEFT; }
    if (texture == TEXTURE_ASSET_ID::BOTTOM_RIGHT_WALL1) { return WALL_DIRECTION::BOTTOM_RIGHT; }
    return WALL_DIRECTION::NONE;
}

void fill_missing_with_floor(Room& room) {
    for (int y = 0; y < room.room_height; y++) {
        for (int x = 0; x < room.room_width; x++) {
            if (room.room_grid[y][x] != -1) {
                continue;
            }

            Tile floor_tile = get_random_floor_tile();
            add_tile_to_room(room, { x, y }, floor_tile);
        }
    }
}

bool add_room_to_map(Map& map, Room& room, ivec2 pos) {
    int pos_x = pos.x;
    int pos_y = pos.y;
    if (pos.x < 0 || pos.x >= map.grid_width || pos.y < 0 || pos.y >= map.grid_height) {
        return false;
    }

    int end_pos_x = pos_x + room.room_width - 1;
    int end_pos_y = pos_y + room.room_height - 1;

    if (end_pos_x >= map.grid_width || end_pos_y >= map.grid_height) {
        return false;
    }

    fill_missing_with_floor(room);
    int room_id = (int)Entity();

    for (const auto& [id, tile] : room.room_tiles) {
        map.tiles.emplace(id, tile);
    }

    for (int i = pos_y; i <= end_pos_y; i++) {
        for (int j = pos_x; j <= end_pos_x; j++) {
            map.room_mask[i][j] = room_id;
            int tile_id = room.room_grid[i - pos_y][j - pos_x];
            map.tile_object_grid[i][j] = tile_id;
            map.tile_id_grid[i][j] = map.tiles[tile_id].id;
            map.wall_directions[i][j] = room.wall_directions[i - pos_y][j - pos_x];
        }
    } 

    for (const auto& [prop_pos, prop] : room.props) {
        map.props[{pos.x + prop_pos.x, pos.y + prop_pos.y}] = prop;
        map.information_props[{pos.x + prop_pos.x, pos.y + prop_pos.y}] = prop;
        map.tile_id_grid[pos.y + prop_pos.y][pos.x + prop_pos.x] = TILE_ID::WALL;
    }

    return true;
}

bool add_door(Map& map, ivec2 pos) {
    int x = pos.x;
    int y = pos.y;
    if (x < 0 || x >= map.grid_width || x < 1 || y >= map.grid_height ) {
        return false;
    }

    map.door_locations.push_back(pos);
    return true;
}

void update_door_tiles(Map& map, ivec2 pos, int size, bool vertical) {
    int x = pos.x;
    int y = pos.y;
    int offset = size / 2;
    Tile door_tile = Tile();
    door_tile.id = TILE_ID::CLOSED_DOOR;
    door_tile.size = { 1, 1 };
    door_tile.texture = TEXTURE_ASSET_ID::FLOOR_1;
    door_tile.normal = TEXTURE_ASSET_ID::FLOOR_1_NORMAL;
    if (vertical) {
		for (int i = 0; i < size; i++) {
            set_tile(map, { x, y - offset + i }, door_tile);
		}
	} else {
		for (int i = 0; i < size; i++) {
            set_tile(map, { x - offset + i, y }, door_tile);
		}
	}
}

void add_light(Map& map, vec2 pos, vec3 color, float radius, float intensity, float height) {
    Light light = Light{ color, pos, radius, intensity, false, height };
    map.lights.push_back(light);
}

void add_temp_light(vec2 pos, vec3 color, float radius, float intensity, bool is_local, float timer) {
    Entity entity = Entity();
    Light& light = registry.lights.emplace(entity);
    light.color = color;
    light.position = pos;
    light.radius = radius;
    light.intensity = intensity;
    light.is_local = is_local;

    RemoveTimer& removeTimer = registry.removeTimers.emplace(entity);
    removeTimer.remaining_time = timer;
}

void add_pickup_to_map(Map& map, vec2 grid_pos, PICKUP_TYPE pickup_type, int value, GUN_TYPE gun_type) {
    Pickup pickup = Pickup{ grid_pos, pickup_type, gun_type, value, GRID_CELL_SIZE*1.5f };
    map.pickups.push_back(pickup);
}

void add_enemy_to_map(Map& map, ivec2 grid_position, GUN_TYPE gun_type, float health, float speed_factor, float detection_range_factor, float attack_range_factor) {
    EnemyBlueprint enemy_blueprint = { grid_position, gun_type, health, speed_factor, detection_range_factor, attack_range_factor };
    map.enemy_blueprints.push_back(enemy_blueprint);
}

void update_tile_grid(Map& map) {
    for (const auto& pos : map.door_locations) {
        int col = pos.x;
        int row = pos.y;
        bool vertical = true;
        if (get_tile(map, { col - 1, row }) == TILE_ID::WALL && get_tile(map, {col + 1, row}) == TILE_ID::WALL) {
            vertical = false;
        }
        update_door_tiles(map, {col, row}, 3, vertical);
    }
}

void set_tile(Map& map, ivec2 pos, Tile tile) {
    int id = (int)Entity();

    map.tile_id_grid[pos.y][pos.x] = tile.id;
    map.tile_object_grid[pos.y][pos.x] = id;
    map.tiles[id] = tile;
}

TILE_ID get_tile(Map& map, ivec2 pos) {
    return map.tile_id_grid[pos.y][pos.x];
}

Map load_map_from_file(const std::string& name) {
    std::string file_path = std::string(PROJECT_SOURCE_DIR) + "maps/" + name + ".json";
    std::cout << file_path << std::endl;

    FILE* fp = fopen(file_path.c_str(), "r");

    char readBuffer[65536]; 
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer)); 
    rapidjson::Document doc; 
    doc.ParseStream(is); 
    fclose(fp);

	int width = doc["width"].GetInt();
    int height = doc["height"].GetInt();

    Map map = create_map_struct(width, height);

    int base_tileset_firstgid;
    int props_tileset_firstgid;
    int doors_tileset_firstgid;

    std::vector<Light> lights_layer;

    std::vector<TilesetInfo> tilesets;
	const rapidjson::Value& tilesets_array = doc["tilesets"];
	
	for (rapidjson::SizeType i = 0; i < tilesets_array.Size(); i++) {
		const rapidjson::Value& tileset_obj = tilesets_array[i];
		
		TilesetInfo info;
		info.firstgid = tileset_obj["firstgid"].GetInt();
        std::string source = tileset_obj["source"].GetString();
        size_t last_slash = source.find_last_of("/\\");
        std::string filename = source.substr(last_slash + 1);
        size_t dot_position = filename.find_last_of(".");
        info.name = filename.substr(0, dot_position);
		
		tilesets.push_back(info);
	}

    const rapidjson::Value& layers = doc["layers"];
    const rapidjson::Value& base_data = layers[0]["data"];
    const rapidjson::Value& props_data = layers[1]["data"];
    const rapidjson::Value& light_objects = layers[2]["objects"];
    const rapidjson::Value& room_objects = layers[3]["objects"];

	for (rapidjson::SizeType i = 0; i < base_data.Size(); i++) {
		int x = i % width;
		int y = i / width;
        int gid = base_data[i].GetInt();
        if (gid == 0) {
            continue;
        }
        auto [tileset_name, lid] = global_to_local_id(gid, tilesets);
		Tile tile = get_tile_from_tileset(tileset_name, lid);
		int tile_id = (int)Entity();
        map.tiles.emplace(tile_id, tile);
		map.tile_object_grid[y][x] = tile_id;
		map.tile_id_grid[y][x] = map.tiles[tile_id].id;
        map.wall_directions[y][x] = get_wall_direction(tile.texture);
	}

	for (rapidjson::SizeType i = 0; i < props_data.Size(); i++) {
		int x = i % width;
		int y = i / width;
		int gid = props_data[i].GetInt();
        if (gid == 0) {
            continue;
        }
        auto [tileset_name, lid] = global_to_local_id(gid, tilesets);
        Prop prop = get_prop_from_tileset(tileset_name, lid);
        map.props[{x, y}] = prop;
	}

	for (rapidjson::SizeType i = 0; i < light_objects.Size(); i++) {
		const rapidjson::Value& obj = light_objects[i];
		
		Light light;
        light.height = GRID_CELL_SIZE;
		
		float x = obj["x"].GetFloat() / 128.;
		float y = obj["y"].GetFloat() / 128.;
        light.position = grid_to_world_coord(x, y);
		
		const rapidjson::Value& props = obj["properties"];
		
		for (rapidjson::SizeType j = 0; j < props.Size(); j++) {
			const rapidjson::Value& prop = props[j];
			
			std::string propName = prop["name"].GetString();
			
			if (propName == "radius") {
				light.radius = prop["value"].GetFloat() * GRID_CELL_SIZE;
			}
			else if (propName == "color") {
                std::string hex_color = prop["value"].GetString();
				float b = std::stoi(hex_color.substr(1, 2), nullptr, 16);
				float r = std::stoi(hex_color.substr(3, 2), nullptr, 16);
				float g = std::stoi(hex_color.substr(5, 2), nullptr, 16);
				light.color = vec3(r / 255.0f, g / 255.0f, b / 255.0f);
			}
		}
		
		map.lights.push_back(light);
	}

    for (rapidjson::SizeType i = 0; i < room_objects.Size(); i++) {
		const rapidjson::Value& obj = room_objects[i];
        int start_x = obj["x"].GetInt() / 128.;
        int start_y = obj["y"].GetInt() / 128.;
        int room_height = obj["height"].GetInt() / 128.;
        int room_width = obj["width"].GetInt() / 128.;
        int id = std::stoi(obj["name"].GetString());
        for (int y = start_y; y < start_y + room_height; y++) {
            for (int x = start_x; x < start_x + room_width; x++) {
                map.room_mask[y][x] = id;
            }
        }
    }

    return map;
};

std::pair<std::string, int> global_to_local_id(int gid, const std::vector<TilesetInfo>& tilesets) {
	int tileset_index = -1;
    int max_first_gid = 0;
    
    for (size_t i = 0; i < tilesets.size(); i++) {
        if (tilesets[i].firstgid <= gid && tilesets[i].firstgid > max_first_gid) {
            max_first_gid = tilesets[i].firstgid;
            tileset_index = static_cast<int>(i);
        }
    }
    
	int localId = gid - tilesets[tileset_index].firstgid;
	return {tilesets[tileset_index].name, localId};
}

const Tile get_tile_from_tileset(const std::string& tileset_name, int lid) {
    if (tileset_name == "Base Tiles") {
        auto it = BASE_TILES_ID_MAP.find(lid);
        return it->second;
    }
    else {
		return Tile{};
    }
}

const Prop get_prop_from_tileset(const std::string& tileset_name, int lid) {
    if (tileset_name == "Props and Others") {
        auto it = PROPS_AND_OTHERS_ID_MAP.find(lid);
        return it->second;
    }
    else if (tileset_name == "Door frame and lights") {
        auto it = DOOR_FRAME_AND_LIGHTS_ID_MAP.find(lid);
        return it->second;
    }
    else {
        return Prop{};
    }
}

void create_map_0() {
    Map map_struct = create_map_struct(50, 100);
    Entity map_entity = Entity();
    Map& map = registry.maps.insert(map_entity, map_struct);

    map.start_location = {16, 28};
    map.music = MUSIC_ASSET_ID::MUSIC1;
    map.gun_type = GUN_TYPE::PISTOL;
    map.level_title = TEXTURE_ASSET_ID::LEVEL_0_TEXT;
	
    Room room1 = create_room(11, 7, true); // Left room at the top
    add_tile_to_room(room1, { 10, 4 }, get_random_floor_tile());
    add_tile_to_room(room1, { 10, 5 }, get_random_floor_tile());
    add_tile_to_room(room1, { 10, 6 }, BOTTOM_WALL1);
	add_room_to_map(map, room1, { 0, 0 });

	Room room2 = create_room(10, 7, true); // Middle room at the top
    add_tile_to_room(room2, { 0, 4 }, get_random_floor_tile());
    add_tile_to_room(room2, { 0, 5 }, get_random_floor_tile());
    add_tile_to_room(room2, { 0, 6 }, BOTTOM_WALL1);

    add_tile_to_room(room2, { 9, 0 }, TOP_WALL1);
    add_tile_to_room(room2, { 9, 1 }, get_random_floor_tile());
    add_tile_to_room(room2, { 9, 2 }, get_random_floor_tile());

	add_room_to_map(map, room2, { 11, 0 });

	Room room3 = create_room(13, 7, true); // Right room at the top
    add_tile_to_room(room3, { 0, 0 }, TOP_WALL1);
    add_tile_to_room(room3, { 0, 1 }, get_random_floor_tile());
    add_tile_to_room(room3, { 0, 2 }, get_random_floor_tile());
	add_room_to_map(map, room3, { 21, 0 });

	Room room4 = create_room(7, 17, true);
	add_room_to_map(map, room4, { 0, 7 });

	Room room5 = create_room(19, 17, true); // Center section
	add_room_to_map(map, room5, { 7, 7 });
    
	Room room6 = create_room(7, 17, true); // Right hallway
	add_room_to_map(map, room6, { 26, 7 });

	Room spawn_room = create_room(9, 58, true); // Starting room
    add_prop_to_room(spawn_room, CHEST_BASE, { 2, 46 });
    add_prop_to_room(spawn_room, CHEST0, { 2, 48 });
    add_prop_to_room(spawn_room, CHEST_DISABLED, { 2, 50 });
    add_prop_to_room(spawn_room, CHEST_ENABLED, { 2, 52 });
    add_prop_to_room(spawn_room, CHEST_OPEN, { 2, 54 });
    add_prop_to_room(spawn_room, CONTAINER, { 6, 46 });
    add_prop_to_room(spawn_room, CONTAINER_DISABLED, { 6, 48 });
    add_prop_to_room(spawn_room, SERVER, { 6, 50 });
    add_prop_to_room(spawn_room, SERVER_DISABLED, { 6, 52 });
    add_prop_to_room(spawn_room, SERVER_DOWN, { 6, 54 });
    add_prop_to_room(spawn_room, BLUE_LIGHT_TOP, { 0, 49 });
    add_prop_to_room(spawn_room, BLUE_LIGHT_VERTICAL, { 0, 50 });
    add_prop_to_room(spawn_room, BLUE_LIGHT_VERTICAL, { 0, 51 });
    add_prop_to_room(spawn_room, BLUE_LIGHT_BOTTOM, { 0, 52 });
    add_prop_to_room(spawn_room, RED_LIGHT_TOP, { 8, 49 });
    add_prop_to_room(spawn_room, RED_LIGHT_VERTICAL, { 8, 50 });
    add_prop_to_room(spawn_room, RED_LIGHT_VERTICAL, { 8, 51 });
    add_prop_to_room(spawn_room, RED_LIGHT_BOTTOM, { 8, 52 });
    add_prop_to_room(spawn_room, BLUE_LIGHT_LEFT, { 5, 57 });
    add_prop_to_room(spawn_room, BLUE_LIGHT, { 6, 57 });
    add_prop_to_room(spawn_room, BLUE_LIGHT_RIGHT, { 7, 57 });
    add_prop_to_room(spawn_room, RED_LIGHT_LEFT, { 1, 57 });
    add_prop_to_room(spawn_room, RED_LIGHT_HORIZONTAL, { 2, 57 });
    add_prop_to_room(spawn_room, RED_LIGHT_RIGHT, { 3, 57 });
	add_room_to_map(map, spawn_room, { 12, 24 });

	Room room8 = create_room(9, 7, true);
	add_room_to_map(map, room8, { 12, 11 });

    add_door(map, { 26, 19 });
    add_door(map, { 25, 19 });
    add_door(map, { 6, 19 });
    add_door(map, { 7, 19 });
    add_door(map, { 3, 6 });
    add_door(map, { 3, 7});
    add_door(map, { 29, 6 });
    add_door(map, { 29, 7 });
    add_door(map, { 16, 23 });
    add_door(map, { 16, 24 });
    add_door(map, { 16, 11 });

    // add wide doors 
    add_door_prop(map, WIDE_DOOR_HOR, { 16, 24 });
    add_door_prop(map, WIDE_DOOR_VER, { 6, 19 });
    add_door_prop(map, WIDE_DOOR_VER, { 25, 19 });
    add_door_prop(map, WIDE_DOOR_HOR, { 3, 7 });
    add_door_prop(map, WIDE_DOOR_HOR, { 29, 7 });

    // add narrow doors
    add_door_prop(map, NARROW_DOOR_VER, { 10, 5 });
    add_door_prop(map, NARROW_DOOR_VER, { 20, 2 });

    add_light(map, grid_to_world_coord(16, 14), HOT_PINK, 350, 1.5f, 64.);
    add_light(map, grid_to_world_coord(16, 3), ACID_GREEN, 800, 1.f, 64.);
    add_light(map, grid_to_world_coord(3, 14), AMBER_ORANGE, 1000, 1.f, 64.);
    add_light(map, grid_to_world_coord(24, 14), COOL_NEON, 550, 1.f, 128.);
    add_light(map, grid_to_world_coord(10, 13), {0.2f, 0.5f, 0.1f}, 550, 1.f, 64.);
    add_light(map, grid_to_world_coord(16, 28), DEEP_RED, 500, 1.f, 256.);
    add_light(map, grid_to_world_coord(16, 56), DEEP_RED, 2200, 1.f, 1200.);

    add_light(map, grid_to_world_coord(18, 81), COOL_NEON, GRID_CELL_SIZE * 6, 1.f, GRID_CELL_SIZE);
    add_light(map, grid_to_world_coord(14, 81), DEEP_RED, GRID_CELL_SIZE * 6, 1.f, GRID_CELL_SIZE);

    add_light(map, grid_to_world_coord(12, 75.5), COOL_NEON, GRID_CELL_SIZE * 10, 1.f, 32.f);
    add_light(map, grid_to_world_coord(20, 75.5), DEEP_RED, GRID_CELL_SIZE * 10, 1.f, 32.f);

    add_pickup_to_map(map, {5, 1}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);
    add_pickup_to_map(map, {26, 2}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);
    add_pickup_to_map(map, {16, 48}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT); 

    // Main hall
    add_enemy_to_map(map, {22, 19}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {8, 13}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {22, 12}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

	// Left room
    add_enemy_to_map(map, {4, 14}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {5, 9}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {2, 12}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

	// Top left room
    add_enemy_to_map(map, {9, 2}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

	// Top room
    add_enemy_to_map(map, {12, 1}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {19, 5}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

	// Top right room
    add_enemy_to_map(map, {24, 4}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {32, 4}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

	// Right room
    add_enemy_to_map(map, {27, 9}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {30, 13}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

	// tutorial room
    add_enemy_to_map(map, {18, 55}, GUN_TYPE::DUMMY_GUN, 200.f, 5.f, 4.3f, 2.6f);
    add_enemy_to_map(map, {14, 55}, GUN_TYPE::DUMMY_GUN, 200.f, 5.f, 4.3f, 2.6f);
}

void create_map_1() {
    Map map_struct = create_map_struct(50, 100);
    Entity map_entity = Entity();
    Map& map = registry.maps.insert(map_entity, map_struct);

    map.start_location = {22, 43};
    map.music = MUSIC_ASSET_ID::MUSIC2;
    map.gun_type = GUN_TYPE::REVOLVER;
    map.level_title = TEXTURE_ASSET_ID::LEVEL_1_TEXT;

    // Covers entire map
    Room room1 = create_room(35, 30, true);
    add_room_to_map(map, room1, { 8, 10 });
    // Left large room
    add_prop_to_map(map, SERVER_DISABLED, { 10, 34 });
    add_prop_to_map(map, SERVER, { 10, 35 });
    add_prop_to_map(map, SERVER, { 10, 36 });
    add_prop_to_map(map, SERVER_DISABLED, { 10, 37 });
    add_prop_to_map(map, SERVER, { 10, 38 });
    
    add_prop_to_map(map, SERVER, { 13, 34 });
    add_prop_to_map(map, SERVER, { 13, 35 });
    add_prop_to_map(map, SERVER, { 13, 36 });
    add_prop_to_map(map, SERVER_DISABLED, { 13, 37 });
    add_prop_to_map(map, SERVER, { 13, 38 });

    add_prop_to_map(map, SERVER, { 16, 34 });
    add_prop_to_map(map, SERVER_DISABLED, { 16, 35 });
    add_prop_to_map(map, SERVER, { 16, 36 });
    add_prop_to_map(map, SERVER, { 16, 37 });
    add_prop_to_map(map, SERVER, { 16, 38 });

    // top left room 2
    add_prop_to_map(map, SERVER, { 16, 11 });
    add_prop_to_map(map, SERVER, { 17, 11 });
    add_prop_to_map(map, SERVER, { 18, 11 });
    add_prop_to_map(map, SERVER_DISABLED, { 19, 11 });
    add_prop_to_map(map, SERVER_DISABLED, { 20, 11 });
    add_prop_to_map(map, SERVER_DISABLED, { 21, 11 });

    add_prop_to_map(map, SERVER_DISABLED, { 17, 21 });
    add_prop_to_map(map, SERVER_DISABLED, { 17, 22 });
    add_prop_to_map(map, SERVER_DISABLED, { 19, 21 });
    add_prop_to_map(map, SERVER, { 19, 22 });
    add_prop_to_map(map, SERVER, { 21, 21 });
    add_prop_to_map(map, SERVER, { 21, 22 });

    // bottom right room
    add_prop_to_map(map, SERVER, { 41, 33 });
    add_prop_to_map(map, SERVER, { 41, 34 });
    add_prop_to_map(map, SERVER, { 41, 35 });
    add_prop_to_map(map, SERVER, { 41, 36 });
    add_prop_to_map(map, SERVER, { 41, 37 });

    add_prop_to_map(map, SERVER, { 28, 38 });
    add_prop_to_map(map, SERVER, { 29, 38 });
    add_prop_to_map(map, SERVER, { 30, 38 });
    add_prop_to_map(map, SERVER, { 31, 38 });
    add_prop_to_map(map, SERVER, { 32, 38 });
    add_prop_to_map(map, SERVER, { 33, 38 });

    add_prop_to_map(map, SERVER_DISABLED, { 34, 34 });
    add_prop_to_map(map, SERVER_DISABLED, { 35, 34 });
    add_prop_to_map(map, SERVER_DISABLED, { 35, 35 });

    // top right room
    add_prop_to_map(map, SERVER, { 36, 11 });
    add_prop_to_map(map, SERVER, { 37, 11 });
    add_prop_to_map(map, SERVER, { 38, 11 });
    add_prop_to_map(map, SERVER, { 39, 11 });
    add_prop_to_map(map, SERVER, { 40, 11 });
    add_prop_to_map(map, SERVER_DISABLED, { 41, 11 });
    add_prop_to_map(map, SERVER, { 36, 15 });
    add_prop_to_map(map, SERVER, { 37, 15 });
    add_prop_to_map(map, SERVER_DISABLED, { 38, 15 });
    add_prop_to_map(map, SERVER, { 39, 15 });
    add_prop_to_map(map, SERVER, { 40, 15 });
    add_prop_to_map(map, SERVER, { 41, 15 });

    add_prop_to_map(map, SERVER_DISABLED, { 31, 13 });
    add_prop_to_map(map, SERVER_DISABLED, { 31, 14 });
    add_prop_to_map(map, SERVER, { 31, 15 });
    add_prop_to_map(map, SERVER_DISABLED, { 31, 16 });

    add_prop_to_map(map, SERVER, { 31, 22 });
    add_prop_to_map(map, SERVER, { 32, 22 });
    add_prop_to_map(map, SERVER, { 33, 22 });
    add_prop_to_map(map, SERVER, { 34, 22 });
    add_prop_to_map(map, SERVER, { 35, 22 });

    // hallway
    add_prop_to_map(map, SERVER_DISABLED, { 31, 28 });
    add_prop_to_map(map, SERVER_DISABLED, { 32, 28 });
    add_prop_to_map(map, SERVER_DISABLED, { 31, 29 });
    add_prop_to_map(map, SERVER_DISABLED, { 32, 29 });
    
    add_prop_to_map(map, SERVER_DISABLED, { 20, 25 });
    add_prop_to_map(map, SERVER_DISABLED, { 21, 25 });

    add_prop_to_map(map, SERVER_DISABLED, { 20, 33 });
    add_prop_to_map(map, SERVER_DISABLED, { 20, 34 });
    add_prop_to_map(map, SERVER_DISABLED, { 20, 35 });
    add_prop_to_map(map, SERVER_DISABLED, { 21, 34 });

    Room hallway1 = create_room(7, 16, true);
    add_room_to_map(map, hallway1, { 19, 24 });

    Room hallway2 = create_room(18, 7, true);
    add_room_to_map(map, hallway2, { 25, 24 });

    Room hallway3 = create_room(7, 15, true);
    add_room_to_map(map, hallway3, { 23, 10 });

    // Entry room
    Room room2 = create_room(7, 7, true);
    add_room_to_map(map, room2, { 19, 40 });
    add_door(map, { 22, 39 });
    add_door(map, { 22, 40 });
    add_light(map, grid_to_world_coord(22, 42), AMBER_ORANGE, 300, 1.f, 64);

    // Bottom left room
    Room room3 = create_room(11, 16, true);
    add_room_to_map(map, room3, { 8, 24 });
    add_door(map, { 19, 30 });
    add_door(map, { 18, 30 });
    add_light(map, grid_to_world_coord(14, 31), COOL_NEON, 300, 1.f, 64);

    // Bottom right room
    Room room4 = create_room(17, 9, true);
    add_room_to_map(map, room4, { 26, 31 });
    add_door(map, { 25, 34 });
    add_door(map, { 26, 34 });
    add_light(map, grid_to_world_coord(34, 35), COOL_NEON, 500, 1.f, 64);

    // Top right room
    Room room5 = create_room(13, 14, true);
    add_room_to_map(map, room5, { 30, 10 });
    add_door(map, { 39, 23 });
    add_door(map, { 39, 24 });
    add_light(map, grid_to_world_coord(36, 17), COOL_NEON, 500, 1.f, 64);

    // Top left 1 (washroom)
    Room room6 = create_room(7, 14, true);
    add_room_to_map(map, room6, { 8, 10 });
    add_door(map, { 11, 24 });
    add_door(map, { 11, 23 });
    add_light(map, grid_to_world_coord(11, 16), AMBER_ORANGE, 300, 1.f, 64);

    // Top left 2
    Room room7 = create_room(8, 14, true);
    add_room_to_map(map, room7, { 15, 10 });
    add_door(map, { 23, 18 });
    add_door(map, { 22, 18 });
    add_light(map, grid_to_world_coord(19, 16), COOL_NEON, 400, 1.f, 64); 

    // Exit
    Room room8 = create_room(7, 7, true);
    add_room_to_map(map, room8, { 23, 3 });
    add_door(map, { 26, 10 });
    add_door(map, { 26, 9 });
    add_light(map, grid_to_world_coord(26, 7), AMBER_ORANGE, 300, 1.f, 64);

    // Hallway lights
    add_light(map, grid_to_world_coord(22, 32), AMBER_ORANGE, 300, 1.f, 128.);
    add_light(map, grid_to_world_coord(27, 26), AMBER_ORANGE, 300, 1.f, 128.);
    add_light(map, grid_to_world_coord(35, 27), AMBER_ORANGE, 300, 1.f, 128.);
    add_light(map, grid_to_world_coord(26, 16), AMBER_ORANGE, 300, 1.f, 128.);

    // Delete walls in the hallways
    set_tile(map, { 24, 24 }, get_random_floor_tile());
    set_tile(map, { 25, 24 }, get_random_floor_tile());
    set_tile(map, { 25, 24 }, get_random_floor_tile());
    set_tile(map, { 26, 24 }, get_random_floor_tile());
    set_tile(map, { 27, 24 }, get_random_floor_tile());
    set_tile(map, { 28, 24 }, get_random_floor_tile());
    set_tile(map, { 25, 25 }, get_random_floor_tile());
    set_tile(map, { 25, 26 }, get_random_floor_tile());
    set_tile(map, { 25, 27 }, get_random_floor_tile());
    set_tile(map, { 25, 28 }, get_random_floor_tile());
    set_tile(map, { 25, 29 }, get_random_floor_tile());

    // Set corner walls
    set_tile(map, { 25, 30 }, TOP_LEFT_WALL_CORNER1);
    set_tile(map, { 23, 24 }, BOTTOM_RIGHT_WALL_CORNER1);
    set_tile(map, { 29, 24 }, BOTTOM_LEFT_WALL_CORNER1);

    // Pickups
    add_pickup_to_map(map, {39, 34}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);
    add_pickup_to_map(map, {11, 13}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);
    add_pickup_to_map(map, {38, 13}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);

    //Enemies
    // Bottom left room
    add_enemy_to_map(map, {14, 27}, GUN_TYPE::REVOLVER, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {11, 31}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {12, 35}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Bottom right room
    add_enemy_to_map(map, {30, 36}, GUN_TYPE::REVOLVER, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {34, 33}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {38, 37}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Top right room
    add_enemy_to_map(map, {35, 19}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {38, 16}, GUN_TYPE::REVOLVER, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {33, 21}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    // Top left room 1 (washroom)
    add_enemy_to_map(map, {11, 19}, GUN_TYPE::REVOLVER, 200.f, 5.f, 20.f, 10.f);

    // Top left room 2
    add_enemy_to_map(map, {20, 13}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {18, 19}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Hallway
    add_enemy_to_map(map, {36, 27}, GUN_TYPE::REVOLVER, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {22, 27}, GUN_TYPE::REVOLVER, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {26, 14}, GUN_TYPE::REVOLVER, 200.f, 5.f, 20.f, 10.f);

    // add door prop
    add_door_prop(map, WIDE_DOOR_HOR, { 22, 40 });
    add_door_prop(map, WIDE_DOOR_VER, { 18, 30 });
    add_door_prop(map, WIDE_DOOR_VER, { 25, 34 });
    add_door_prop(map, WIDE_DOOR_HOR, { 11, 24 });
    add_door_prop(map, WIDE_DOOR_HOR, { 39, 24 });
    add_door_prop(map, WIDE_DOOR_HOR, { 26, 10 });
    add_door_prop(map, WIDE_DOOR_VER, { 22, 18 });
}

void create_map_2() {
    Map file_map = load_map_from_file("level2");
    Entity map_entity = Entity();
    Map& map = registry.maps.insert(map_entity, file_map);

    map.start_location = {18, 43};
    map.music = MUSIC_ASSET_ID::MUSIC3;
    map.gun_type = GUN_TYPE::SHOTGUN;
    map.level_title = TEXTURE_ASSET_ID::LEVEL_2_TEXT;

    // Bottom room
    add_enemy_to_map(map, {12, 37}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {23, 34}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Bottom Left 
    add_enemy_to_map(map, {11, 29}, GUN_TYPE::SMG, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {6, 29}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 9.f, 6.f);

    // Bottom right
    add_enemy_to_map(map, {30, 36}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {29, 31}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 9.f, 6.f);
    
    // Top left
    add_enemy_to_map(map, {5, 19}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 9.f, 6.f);
    add_enemy_to_map(map, {9, 13}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Middle right 
    add_enemy_to_map(map, {33, 25}, GUN_TYPE::SMG, 200.f, 5.f, 9.f, 6.f);
    add_enemy_to_map(map, {29, 21}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Centre room 1
    add_enemy_to_map(map, {23, 29}, GUN_TYPE::SMG, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {17, 26}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 9.f, 6.f);

    // Centre room 2
    add_enemy_to_map(map, {12, 23}, GUN_TYPE::SMG, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {15, 19}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Top right
    add_enemy_to_map(map, {33, 12}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 9.f, 6.f);

    // Top room
    add_enemy_to_map(map, {17, 14}, GUN_TYPE::SMG, 200.f, 5.f, 20.f, 10.f);
    add_enemy_to_map(map, {25, 12}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 9.f, 6.f);
    add_enemy_to_map(map, {22, 20}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 20.f, 10.f);

    // Health boxes
    add_pickup_to_map(map, {12, 23}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);
    add_pickup_to_map(map, {33, 24}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);
    add_pickup_to_map(map, {5, 12}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);

    // wide doors
    add_door_prop(map, WIDE_DOOR_HOR, { 18, 40 });
    add_door_prop(map, WIDE_DOOR_HOR, { 21, 10 });

    // narrow doors
    add_door_prop(map, NARROW_DOOR_HOR, { 17, 32 });
    add_door_prop(map, NARROW_DOOR_VER, { 8, 37 });
    add_door_prop(map, NARROW_DOOR_VER, { 25, 37 });
    add_door_prop(map, NARROW_DOOR_HOR, { 22, 24 });
    add_door_prop(map, NARROW_DOOR_HOR, { 28, 28 });
    add_door_prop(map, NARROW_DOOR_HOR, { 32, 19 });
    add_door_prop(map, NARROW_DOOR_VER, { 27, 16 });
    add_door_prop(map, NARROW_DOOR_VER, { 18, 21 });
    add_door_prop(map, NARROW_DOOR_VER, { 8, 20 });
    add_door_prop(map, NARROW_DOOR_HOR, { 5, 24 });
    add_door_prop(map, NARROW_DOOR_VER, { 12, 13 });
}

void create_map_3() {
    Map file_map = load_map_from_file("level3");
    Entity map_entity = Entity();
    Map& map = registry.maps.insert(map_entity, file_map);

    map.start_location = {25, 46};
    map.music = MUSIC_ASSET_ID::MUSIC4;
    map.gun_type = GUN_TYPE::SMG;
    map.level_title = TEXTURE_ASSET_ID::LEVEL_3_TEXT;


    // Enemies
    // Centre hallway
    add_enemy_to_map(map, {25, 33}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 10.f, 8.f);
    add_enemy_to_map(map, {25, 23}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 10.f, 8.f);
    add_enemy_to_map(map, {25, 12}, GUN_TYPE::SMG, 200.f, 5.f, 10.f, 8.f);

    // Right hallway
    add_enemy_to_map(map, {36, 39}, GUN_TYPE::SMG, 200.f, 5.f, 10.f, 8.f);
    
    // Left hallway
    add_enemy_to_map(map, {11, 39}, GUN_TYPE::REVOLVER, 200.f, 5.f, 10.f, 8.f);

    // Bottom left left (3)
    add_enemy_to_map(map, {11, 26}, GUN_TYPE::SMG, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {6, 30}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 12.f, 9.f);

    // Bottom left right (2)
    add_enemy_to_map(map, {18, 26}, GUN_TYPE::SMG, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {18, 31}, GUN_TYPE::RAILGUN, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {14, 33}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 12.f, 9.f);

    // Top left left (5)
    add_enemy_to_map(map, {6, 10}, GUN_TYPE::RAILGUN, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {11, 11}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 12.f, 9.f);

    // Top left right (6)
    add_enemy_to_map(map, {18, 11}, GUN_TYPE::SMG, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {18, 18}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 12.f, 9.f);

    // Center left (4)
    add_enemy_to_map(map, {6, 20}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {11, 17}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 12.f, 9.f);

    // Bottom right right (8)
    add_enemy_to_map(map, {44, 29}, GUN_TYPE::SMG, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {40, 32}, GUN_TYPE::REVOLVER, 200.f, 5.f, 12.f, 9.f);

    // Bottom right left (7)
    add_enemy_to_map(map, {35, 26}, GUN_TYPE::SMG, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {32, 33}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 12.f, 9.f);

    // Center right left (11)
    add_enemy_to_map(map, {32, 19}, GUN_TYPE::RAILGUN, 200.f, 5.f, 12.f, 9.f);
    add_enemy_to_map(map, {36, 17}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 12.f, 9.f);

    // Center right right (10)
    add_enemy_to_map(map, {43, 20}, GUN_TYPE::RAILGUN, 200.f, 5.f, 12.f, 9.f);

    // Top right left (12)
    add_enemy_to_map(map, {43, 10}, GUN_TYPE::ENEMY_PISTOL, 200.f, 5.f, 12.f, 9.f);

    // Top right right (13)
    add_enemy_to_map(map, {34, 9}, GUN_TYPE::SHOTGUN, 200.f, 5.f, 12.f, 9.f);


    // Health boxes
    add_pickup_to_map(map, {44, 40}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);    // Right hallway
    add_pickup_to_map(map, {5, 40}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);     // Left hallway
    add_pickup_to_map(map, {5, 9}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);      // Top left
    add_pickup_to_map(map, {44, 9}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);     // Top right
    add_pickup_to_map(map, {19, 25}, PICKUP_TYPE::HEALTH_BOX, 300, GUN_TYPE::GUN_COUNT);    // Bottom left
}