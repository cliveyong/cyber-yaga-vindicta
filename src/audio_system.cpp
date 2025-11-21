#include "world_system.hpp"
#include "audio_system.hpp"

// stlib
#include <vector>
#include <iostream>

// create the world
void AudioSystem::init() {
	bool loading_successful = start_and_load_sounds();
	if (!loading_successful) {
		fprintf(stderr, "Failed to initialize audio\n");
	}
}

bool AudioSystem::start_and_load_sounds() {
	
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio\n");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device\n");
		return false;
	}

	loaded_sounds.reserve(sound_count);
	loaded_musics.reserve(music_count);

	for (std::string sound_path: sound_paths) {
		Mix_Chunk* loaded_sound = Mix_LoadWAV(sound_path.c_str());
		if (loaded_sound == nullptr) {
			fprintf(stderr, "Failed to load some sounds. make sure the data directory is present\n");
			return false;
		}
		loaded_sounds.push_back(loaded_sound);
	}

	for (std::string music_path: music_paths) {
		Mix_Music* loaded_music = Mix_LoadMUS(music_path.c_str());
		if (loaded_music == nullptr) {
			fprintf(stderr, "Failed to load some musics. make sure the data directory is present\n");
			return false;
		}
		loaded_musics.push_back(loaded_music);
	}

	return true;
}

void AudioSystem::play_sound(SOUND_ASSET_ID sound_id, int volume) {
	Mix_Chunk* sound = loaded_sounds[(int) sound_id];
	if (sound != nullptr) {
		Mix_VolumeChunk(sound, volume);
		Mix_PlayChannel(-1, sound, 0);
	}
}

void AudioSystem::play_music(MUSIC_ASSET_ID music_id, int volume) {
	Mix_Music* music = loaded_musics[(int) music_id];
	if (music != nullptr) {
		Mix_VolumeMusic(volume);
		Mix_PlayMusic(music, -1);
	}
}

AudioSystem::~AudioSystem() {
	for (Mix_Chunk* sound: loaded_sounds) {
		if (sound != nullptr) {
			Mix_FreeChunk(sound);
		}
	}
	for (Mix_Music* music: loaded_musics) {
		if (music != nullptr) {
			Mix_FreeMusic(music);
		}
	}
	Mix_CloseAudio();
}