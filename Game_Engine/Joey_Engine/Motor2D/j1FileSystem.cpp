#include "j1FileSystem.h"
#include "p2Log.h"
#include "PhysFS/include/physfs.h"
#include "SDL/include/SDL.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

j1FileSystem::j1FileSystem(const char* game_path) : j1Module()
{
	name.create("filesystem");
	
	if (PHYSFS_init(NULL) == 0)
		LOG("Failed to initialize File System : %s", PHYSFS_getLastError());

	AddPath(game_path);
}

j1FileSystem::~j1FileSystem()
{
	PHYSFS_deinit();
}

bool j1FileSystem::Awake()
{
	LOG("Loading File System");

	return true;
}

bool j1FileSystem::CleanUp()
{
	LOG("Freeing File System subsistem");

	return true;
}

bool j1FileSystem::AddPath(const char* path)
{
	bool ret = false;

	if (PHYSFS_mount(path, NULL, 1) != 0)
	{
		LOG("Added path %s to file System", path);
		ret = true;
	}
	else
		LOG("Failed to add path %s to file System", path);

	return ret;
}

int j1FileSystem::LoadFile(const char* path, char** buffer)
{
	int ret = 0;

	PHYSFS_File* file = PHYSFS_openRead(path);

	if (file != NULL)
	{
		PHYSFS_uint32 size = PHYSFS_fileLength(file);
		if (size > 0)
		{
			*buffer =  new char[size];
			PHYSFS_uint32 objects = PHYSFS_read(file, *buffer, 1, size);
			
			if (size != objects)
			{
				LOG("File System error while trying to read %s : %s", path, PHYSFS_getLastError());
				RELEASE(buffer);
			}
			else
				ret = objects;
		}

		if (PHYSFS_close(file) == 0)
			LOG("FileS System error while trying to close %s : %s", path, PHYSFS_getLastError());
	}
	else
		LOG("FileS System error while trying to open %s : %s", path, PHYSFS_getLastError());

	return ret;
}

SDL_RWops* j1FileSystem::LoadFile(const char* path)
{
	char* buffer;
	int size = LoadFile(path, &buffer);

	if (size > 0)
	{
		SDL_RWops* data = SDL_RWFromConstMem(buffer, size);

		if (data != NULL)
		{
			data->close = close_sdl_rwops;
			return data;
		}
		else
		{
			LOG("Failed to convert file data to a SDL_RWops type");
			return NULL;
		}
	}
	else
		return NULL;
}

int close_sdl_rwops(SDL_RWops *rw)
{
	RELEASE(rw->hidden.mem.base);
	SDL_FreeRW(rw);
	return 0;
}