#include "j1FileSystem.h"
#include "j1App.h"
#include "p2Log.h"
#include "PhysFS/include/physfs.h"
#include "SDL/include/SDL.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

j1FileSystem::j1FileSystem() : j1Module()
{
	name.create("filesystem");
	
	char* base_path = SDL_GetBasePath();
	if (PHYSFS_init(base_path) == 0)
		LOG("Failed to initialize File System : %s", PHYSFS_getLastError());
	
	AddPath(".");
	
}

j1FileSystem::~j1FileSystem()
{
	PHYSFS_deinit();
}

bool j1FileSystem::Awake(pugi::xml_node& node)
{
	LOG("Loading File System");

	for (pugi::xml_node path = node.child("path"); path; path = path.next_sibling("path"))
	{
		AddPath(path.attribute("file").as_string());
	}

	// Ask SDL for a write dir
	char* write_path = SDL_GetPrefPath(App->GetCreator(), App->GetTitle());

	if (PHYSFS_setWriteDir(write_path) == 0)
		LOG("File System error while creating write dir: %s\n", PHYSFS_getLastError());
	else
	{
		// We add the writing directory as a reading directory too with speacial mount point
		LOG("Writing directory is %s\n", write_path);
		AddPath(write_path, GetSaveDirectory());
	}

	SDL_free(write_path);

	return true;
}

bool j1FileSystem::CleanUp()
{
	LOG("Freeing File System subsistem");

	return true;
}

bool j1FileSystem::AddPath(const char* path, const char* mount_point)
{
	bool ret = false;

	if (PHYSFS_mount(path,mount_point , 1) != 0)
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

int j1FileSystem::SaveFile(const char* file, const char* buffer, int size) const
{
	int ret = 0;

	PHYSFS_file* fs_file = PHYSFS_openWrite(file);

	if (fs_file != NULL)
	{
		PHYSFS_sint64 written = PHYSFS_write(fs_file, (const void*)buffer, 1, size);
		if (written != size)
			LOG("File System error while writing to file %s: %s\n", file, PHYSFS_getLastError());
		else
			ret = (uint)written;

		if (PHYSFS_close(fs_file) == 0)
			LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
	}
	else
		LOG("File System error while opening file %s: %s\n", file, PHYSFS_getLastError());

	return ret;
}