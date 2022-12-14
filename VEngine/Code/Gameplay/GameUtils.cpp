#include "vpch.h"
#include "GameUtils.h"
#include <filesystem>
#include "Actors/Game/Player.h"
#include "Actors/Game/Grid.h"
#include "Actors/Game/EntranceTrigger.h"
#include "Audio/AudioSystem.h"
#include "World.h"
#include "FileSystem.h"
#include "GameInstance.h"
#include "Camera.h"
#include "Components/CameraComponent.h"
#include "UI/UISystem.h"
#include "UI/ScreenFadeWidget.h"
#include "UI/Game/MemoryTransferWidget.h"
#include "Input.h"
#include "Log.h"
#include "Particle/SpriteSheet.h"
#include "Asset/AssetFileExtensions.h"
#include "Gameplay/BattleSystem.h"

namespace GameUtils
{
	std::string levelToMoveTo;

	bool CheckIfMemoryExists(const std::string memoryName)
	{
		auto memIt = GameInstance::playerMemories.find(memoryName);
		return memIt != GameInstance::playerMemories.end();
	}

	void SetActiveCameraTarget(Actor* newTarget)
	{
		activeCamera->targetActor = newTarget;
	}

	void SetActiveCameraTargetAndZoomIn(Actor* newTarget)
	{
		activeCamera->targetActor = newTarget;
		Player::system.GetFirstActor()->nextCameraFOV = 30.f;
	}
	
	void SetActiveCameraTargetAndZoomOut(Actor* newTarget)
	{
		activeCamera->targetActor = newTarget;
		Player::system.GetFirstActor()->nextCameraFOV = 60.f;
	}

	void SetCameraBackToPlayer()
	{
		auto player = Player::system.GetFirstActor();
		player->nextCameraFOV = 60.f;
		activeCamera->targetActor = player;
	}

	void CameraShake(float shake)
	{
		activeCamera->shakeLevel = shake;
	}

	SpriteSheet* SpawnSpriteSheet(std::string textureFilename, XMFLOAT3 position, bool loop, int numRows, int numColumns)
	{
		auto spriteSheet = SpriteSheet::system.Add("SpriteSheet", nullptr, SpriteSheet(), false);

		spriteSheet->SetPosition(position);
		spriteSheet->textureData.filename = textureFilename;
		spriteSheet->loopAnimation = loop;
		spriteSheet->numSheetRows = numRows;
		spriteSheet->numSheetColumns = numColumns;

		spriteSheet->Create();

		return spriteSheet;
	}

	void PlayAudioOneShot(const std::string audioFilename)
	{
		AudioSystem::PlayAudio(audioFilename);
	}

	void SaveGameWorldState()
	{
		//The deal with save files here is the the .vmap file is the original state of the world seen in-editor
		//and the .sav version of it is based on in-game player saves. So if a .sav version exists, that is loaded
		//instead of the .vmap file, during gameplay.

		auto firstOf = World::worldFilename.find_first_of(".");
		std::string str = World::worldFilename.substr(0, firstOf);
		std::string file = str += AssetFileExtensions::gameSave;

		World::worldFilename = file;
		FileSystem::SerialiseAllSystems();
	}

	void LoadWorld(std::string worldName)
	{
		std::string path = "WorldMaps/" + worldName;

		if (GameInstance::useGameSaves)
		{
			path = "GameSaves/" + worldName;
		}

		GameUtils::SaveGameInstanceData();

		battleSystem.Reset();

		FileSystem::LoadWorld(worldName);
	}

	void SaveGameInstanceData()
	{
		if (GameInstance::useGameSaves)
		{
			Properties instanceProps = GameInstance::GetInstanceSaveData();
			Serialiser s(gameInstanceSaveFile, OpenMode::Out);
			s.Serialise(instanceProps);
		}
	}

	void LoadGameInstanceData()
	{
		if (GameInstance::useGameSaves)
		{
			Properties instanceProps = GameInstance::GetInstanceSaveData();
			Deserialiser d(gameInstanceSaveFile, OpenMode::In);
			d.Deserialise(instanceProps);
		}
	}

	void LoadWorldAndMoveToEntranceTrigger()
	{
		if (levelToMoveTo.empty())
		{
			return;
		}

		LoadWorld(levelToMoveTo);

		UISystem::screenFadeWidget->SetToFadeIn();
		UISystem::screenFadeWidget->AddToViewport();

		int matchingEntranceTriggerCount = 0;
		//Set player pos and rot at entrancetrigger in loaded world with same name as previous.
		auto entranceTriggers = World::GetAllActorsOfTypeInWorld<EntranceTrigger>();
		for (auto entrance : entranceTriggers)
		{
			if (entrance->levelToMoveTo == GameInstance::previousMapMovedFrom)
			{
				matchingEntranceTriggerCount++;

				auto player = Player::system.GetFirstActor();

				player->SetPosition(entrance->GetPosition());
				player->nextPos = player->GetPositionV();

				player->SetRotation(entrance->GetRotationV());
				player->nextRot = player->GetRotationV();
			}
		}

		GameInstance::previousMapMovedFrom = levelToMoveTo;

		assert(matchingEntranceTriggerCount < 2 && "Entrances with same name");

		if (matchingEntranceTriggerCount == 0)
		{
			Log("EntranceTrigger with matching [%s] field not found.", levelToMoveTo.c_str());
		}

		Input::blockInput = false;
	}

	void TriggerGameOver()
	{
		auto player = Player::system.GetFirstActor();
		player->gameOver = true;

		UISystem::CreateWidget<MemoryTransferWidget>()->AddToViewport();

		AudioSystem::StopAllAudio();
		AudioSystem::PlayAudio("game05.wav", true);
	}

	void DisablePlayer()
	{
		auto player = Player::system.GetFirstActor();
		player->SetActive(false);
	}

	void SetActiveCamera(CameraComponent* camera)
	{
		activeCamera = camera;
	}
}
