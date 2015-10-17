#ifndef __j1FILESYSTEM_H__
#define __j1FILESYSTEM_H__

#include "j1Module.h" 

struct SDL_RWops;

int close_sdl_rwops(SDL_RWops *rw);

class j1FileSystem : public j1Module
{
	public:

		//Constructor
		j1FileSystem(const char* game_path);
		
		//Destructor
		virtual ~j1FileSystem();

		//General Module Functions
		bool Awake();
		bool CleanUp();

		//FileSystem Specific Functions
		//Adds a Path to the fileSystem tree
		bool AddPath(const char* path);
		
		int LoadFile(const char* path, char** buffer);
		SDL_RWops* LoadFile(const char* path);
};

#endif	__j1FILESYSTEM_H__