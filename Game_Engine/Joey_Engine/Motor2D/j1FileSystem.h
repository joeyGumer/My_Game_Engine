#ifndef __j1FILESYSTEM_H__
#define __j1FILESYSTEM_H__

#include "j1Module.h" 

struct SDL_RWops;

int close_sdl_rwops(SDL_RWops *rw);

class j1FileSystem : public j1Module
{
	public:

		//Constructor
		j1FileSystem();
		
		//Destructor
		virtual ~j1FileSystem();

		//General Module Functions
		bool Awake(pugi::xml_node& node);
		bool CleanUp();

		//FileSystem Specific Functions
		//Adds a Path to the fileSystem tree
		bool AddPath(const char* path, const char* mount_point = NULL);
		
		int SaveFile(const char* path,const char* buffer, int size)const;
		int LoadFile(const char* path, char** buffer);
		SDL_RWops* LoadFile(const char* path);

		//Utils
		const char* GetSaveDirectory() const
		{
			return "save/";
		}
};

#endif	__j1FILESYSTEM_H__