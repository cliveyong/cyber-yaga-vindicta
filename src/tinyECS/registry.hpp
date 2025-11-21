#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "map_system.hpp"
#include "ui_system.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// IMPORTANT: Add any new CC's below to the registry_list
	ComponentContainer<Motion> motions;
	ComponentContainer<UIButton> buttons;
	ComponentContainer<TitleScreenText> text;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Gun> guns;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<DeadEnemy> deadEnemies;
	ComponentContainer<TutorialEnemy> tutorialEnemies;
	ComponentContainer<PathComponent> pathComponents;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<vec3> colors;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Input> inputs;
	ComponentContainer<Collidable> collidables;
	ComponentContainer<AABB> AABBs;
	ComponentContainer<CircleBound> circlebounds;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Map> maps;
	ComponentContainer<GameProgress> gameProgress;
	ComponentContainer<WalkSoundTimer> walkSoundTimers;
	ComponentContainer<Laser> lasers;
	ComponentContainer<StaticCollidable> staticCollidables;
	ComponentContainer<MovingCollidable> movingCollidables;
	ComponentContainer<MovingCircle> movingCircleCollidables;
	ComponentContainer<MovingSAT> movingSATCollidables;
	ComponentContainer<meshCollidable> meshCollidables;
	ComponentContainer<Dash> dashes;
	ComponentContainer<Melee> melees;
	ComponentContainer<Light> lights;
	ComponentContainer<Pickup> pickups;
	ComponentContainer<Character> characters;
	ComponentContainer<Text> texts;
	ComponentContainer<Reload> reloads;
	ComponentContainer<UI> uis;
	ComponentContainer<InstructionMessage> instructionMessages;
	ComponentContainer<TextureWithoutLighting> textureWithoutLighting;
	ComponentContainer<Animation> animations;
	ComponentContainer<RemoveTimer> removeTimers;
	ComponentContainer<SpatialHash> spatialHashes;
	ComponentContainer<ShadowCaster> shadowCasters;
	ComponentContainer<Prop> props;
	ComponentContainer<Debris> debrises;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&guns);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&colors);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&inputs);
		registry_list.push_back(&collidables);
		registry_list.push_back(&AABBs);
		registry_list.push_back(&circlebounds);
		registry_list.push_back(&cameras);
		registry_list.push_back(&enemies);
		registry_list.push_back(&deadEnemies);
		registry_list.push_back(&tutorialEnemies);
		registry_list.push_back(&maps);
		registry_list.push_back(&gameProgress);
		registry_list.push_back(&staticCollidables);
		registry_list.push_back(&movingCollidables);
		registry_list.push_back(&movingCircleCollidables);
		registry_list.push_back(&movingSATCollidables);
		registry_list.push_back(&meshCollidables);
		registry_list.push_back(&dashes);
		registry_list.push_back(&melees);
		registry_list.push_back(&lights);
		registry_list.push_back(&characters);
		registry_list.push_back(&texts);
		registry_list.push_back(&reloads);
		registry_list.push_back(&pickups);
		registry_list.push_back(&uis);
		registry_list.push_back(&instructionMessages);
		registry_list.push_back(&textureWithoutLighting);
		registry_list.push_back(&animations);
		registry_list.push_back(&removeTimers);
		registry_list.push_back(&spatialHashes);
		registry_list.push_back(&shadowCasters);
		registry_list.push_back(&props);
		registry_list.push_back(&debrises);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}

	ScreenState screen_state;
	std::unordered_map<char, Character> character_map;
	MapSystem* map_system;
	AudioSystem* audio_system;
	UISystem* ui_system;
};

extern ECSRegistry registry;