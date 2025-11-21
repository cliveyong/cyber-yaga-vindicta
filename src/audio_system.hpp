#pragma once

#include "tinyECS/components.hpp"

#include <array>
#include <SDL_mixer.h>

// System responsible for setting up audio
class AudioSystem {	

	// Make sure these paths remain in sync with the associated enumerators (see SOUND_ASSET_ID).
	const std::array<std::string, sound_count> sound_paths = {
		audio_path("pistol_gunshot_1.wav"),
		audio_path("footstep.wav"),
		audio_path("bullet_hit_wall_1.wav"),
		audio_path("bullet_hit_flesh_1.wav"),
		audio_path("dodge_woosh.wav"),
		audio_path("dash.wav"),
		audio_path("player_hit_1.wav"),
		audio_path("enemy_hit_1.wav"),
		audio_path("enemy_hit_2.wav"),
		audio_path("enemy_hit_3.wav"),
		audio_path("enemy_hit_4.wav"),
		audio_path("enemy_hit_5.wav"),
		audio_path("enemy_hit_6.wav"),
		audio_path("enemy_hit_7.wav"),
		audio_path("enemy_hit_8.wav"),
		audio_path("bullet_flyby_1.wav"),
		audio_path("bullet_flyby_2.wav"),
		audio_path("bullet_flyby_3.wav"),
		audio_path("pistol_dry_fire_1.wav"),
		audio_path("pistol_reload_1.wav"),
		audio_path("pistol_load_1.wav"),
		audio_path("pistol_load_2.wav"),
		audio_path("pistol_load_3.wav"),
		audio_path("pistol_load_4.wav"),
		audio_path("pistol_load_5.wav"),
		audio_path("health_boost.wav"),
		audio_path("slash.wav"),
		audio_path("shotgun_shot_1.wav"),
		audio_path("door_breaking.wav"),
		audio_path("revolver_dry_fire_1.wav"),
		audio_path("revolver_shot_1.wav"),
		audio_path("revolver_reload_1.wav"),
		audio_path("revolver_cock_1.wav"),
		audio_path("shotgun_reload_1.wav"),
		audio_path("shotgun_cock_1.wav"),
		audio_path("railgun_reload_1.wav"),
		audio_path("railgun_shot_1.wav"),
	};
	
	// Make sure these paths remain in sync with the associated enumerators (see MUSIC_ASSET_ID).
	const std::array<std::string, music_count> music_paths = {
		audio_path("music1.wav"),
		audio_path("music2.wav"),
		audio_path("music3.wav"),
		audio_path("music4.wav")
	};
	
	std::vector<Mix_Chunk*> loaded_sounds = {};

	std::vector<Mix_Music*> loaded_musics = {};

public:
	
	void init();
	
	bool start_and_load_sounds();

	void play_sound(SOUND_ASSET_ID sound_id, int volume);

	void play_music(MUSIC_ASSET_ID music_id, int volume);

	~AudioSystem();
};
