#include <iostream> 
#include <sstream> 

#include "p2Defs.h"
#include "p2Log.h"

#include "j1Window.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Scene.h"
#include "j1FileSystem.h"
#include "j1Map.h"

#include "j1App.h"


// Constructor
j1App::j1App(int argc, char* args[]) : argc(argc), args(args)
{
	frames = 0;
	
	
	input = new j1Input();
	win = new j1Window();
	render = new j1Render();
	tex = new j1Textures();
	audio = new j1Audio();
	scene = new j1Scene();
	fs = new j1FileSystem();
	map = new j1Map();

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(fs);
	AddModule(input);
	AddModule(win);
	AddModule(tex);
	AddModule(audio);
	AddModule(map);
	AddModule(scene);
	

	// render last to swap buffer
	AddModule(render);
}

// Destructor
j1App::~j1App()
{
	// release modules
	p2List_item<j1Module*>* item = modules.end;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->prev;
	}

	modules.clear();
}

void j1App::AddModule(j1Module* module)
{
	module->Init();
	modules.add(module);
}

// Called before render is available
bool j1App::Awake()
{
	bool ret = false;
	//We load the config node, that is the rood node of config_xml
	config = LoadConfig();
	
	if (config.empty() == false)
	{
		ret = true;
		title.create(config.child("app").child("title").child_value());
		creator.create(config.child("app").child("creator").child_value());
	}

	p2List_item<j1Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		ret = item->data->Awake(config.child(item->data->name.GetString()));
		item = item->next;
	}

	return ret;
}

// Called before the first frame
bool j1App::Start()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		ret = item->data->Start();
		item = item->next;
	}

	return ret;
}

// Called each loop iteration
bool j1App::Update()
{
	bool ret = true;
	PrepareUpdate();

	if(input->GetWindowEvent(WE_QUIT) == true)
		ret = false;

	if(ret == true)
		ret = PreUpdate();

	if(ret == true)
		ret = DoUpdate();

	if(ret == true)
		ret = PostUpdate();

	FinishUpdate();
	return ret;
}

// ---------------------------------------------
void j1App::PrepareUpdate()
{
}

// ---------------------------------------------
void j1App::FinishUpdate()
{
	if (want_to_save)
		SaveGameNow();
	if (want_to_load)
		LoadGameNow();
}

// Call modules before each loop iteration
bool j1App::PreUpdate()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PreUpdate();
	}

	return ret;
}

// Call modules on each loop iteration
bool j1App::DoUpdate()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->Update(dt);
	}

	return ret;
}

// Call modules after each loop iteration
bool j1App::PostUpdate()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PostUpdate();
	}

	return ret;
}

// Called before quitting
bool j1App::CleanUp()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.end;

	while(item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}

	return ret;
}

/*
//Getters of private properties from App
*/

int j1App::GetArgc() const
{
	return argc;
}

const char* j1App::GetArgv(int index) const
{
	if(index < argc)
		return args[index];
	else
		return NULL;
}

const char* j1App::GetTitle() const
{
	return title.GetString();
}

const char* j1App::GetCreator() const
{
	return creator.GetString();
}
//---------------------------------------

pugi::xml_node j1App::LoadConfig()
{
	pugi::xml_node ret;

	char* buffer;
	int size = App->fs->LoadFile("config.xml", &buffer);
	pugi::xml_parse_result result = config_file.load_buffer(buffer, size);
	RELEASE(buffer);

	if (result)
		ret = config_file.child("config");
	else
		LOG("Could not load config.xml, pugi error: %s", result.description());

	return ret;
}

/*
//Load and Save methods
*/
void j1App::LoadGame(const char* file)
{
	load_game.create("%s%s", fs->GetSaveDirectory(), file);
	want_to_load = true;
}

void j1App::SaveGame(const char* file)const
{
	save_game.create(file);
	want_to_save = true;
}

bool j1App::LoadGameNow()
{
	bool ret = false;

	char* buffer;
	int size = App->fs->LoadFile(load_game.GetString(), &buffer);

	if (size > 0)
	{
		pugi::xml_document state;
		pugi::xml_node     root;

		pugi::xml_parse_result result = state.load_buffer(buffer, size);
		RELEASE(buffer);

		if (result != NULL)
		{
			LOG("Loading new game state form state.xml");

			root = state.child("state");
			ret = true;

			p2List_item<j1Module*>* item = modules.start;

			while (item != NULL && ret == true)
			{
				ret = item->data->Load(root.child(item->data->name.GetString()));
				item = item->next;
			}

			if (!ret)
				LOG("Loading process interrupted while loading module %s", item->data->name.GetString());
		}
		else
			LOG("Could not load %s file, pugi error: %s", load_game.GetString() ,result.description());
	}

	want_to_load = false;
	return ret;
}

bool j1App::SaveGameNow() const
{
	bool ret = true;

	LOG("Saving Game State to %s", save_game.GetString());

	pugi::xml_document state;
	pugi::xml_node     root;
	
	root = state.append_child("state");

	p2List_item<j1Module*>* item = modules.start;
	while (item != NULL && ret == true)
	{
		ret = item->data->Save(root.append_child(item->data->name.GetString()));
		item = item->next;
	}

	if (ret)
	{
		std::stringstream stream;
		state.save(stream);

		App->fs->SaveFile(save_game.GetString(), stream.str().c_str(), stream.str().length());
		LOG("... finished saving", save_game.GetString());
	}
	else
		LOG("Save process halted from an error in module %s", (item != NULL) ? item->data->name.GetString() : "unknown");
	
	state.reset();
	want_to_save = false;
	return ret;
}