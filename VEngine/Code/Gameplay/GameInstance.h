#pragma once

#include <string>
#include "Properties.h"

struct Memory;

//Instance holding data over the entirety of the game.
//GameInstane is also used as a global save file of sorts, seperate from .vmaps.
struct GameInstance
{
	//this is to set in the editor to know whether to use map files from WorldMaps/ vs GameSaves/
	inline static bool useGameSaves = false;

	static std::string startingMap;
	inline static std::string previousMapMovedFrom = startingMap;

	//Used when continuing from game save files
	inline static std::string mapToLoadOnContinue;

	//Player stats
	inline static int maxPlayerActionPoints = 10;
	inline static int maxPlayerBullets = 2;

	inline static std::map<std::string, Memory*> playerMemories;
	static void AddPlayerMemory(Memory* memory);
	static void DeletePlayerMemory(const std::string memoryName);
	static void DeletePlayerMemories();

	static Properties GetInstanceSaveData();
};
