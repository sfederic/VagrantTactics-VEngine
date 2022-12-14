#include "vpch.h"
#include "World.h"
#include "VMath.h"
#include "FileSystem.h"
#include "Profile.h"
#include "Core.h"
#include "Timer.h"
#include "Log.h"
#include "Actors/MeshActor.h"
#include "Actors/Game/Player.h"
#include "Actors/Game/Grid.h"
#include "Actors/DirectionalLightActor.h"
#include "Actors/ActorSystemCache.h"
#include "Components/ComponentSystemCache.h"
#include "Components/IComponentSystem.h"
#include "Components/MeshComponent.h"
#include "Render/TextureSystem.h"
#include "Render/MaterialSystem.h"
#include "Render/Material.h"
#include "Audio/AudioSystem.h"
#include "Render/SpriteSystem.h"
#include "UI/UISystem.h"
#include "Gameplay/GameInstance.h"
#include "Physics/PhysicsSystem.h"

std::string World::worldFilename;

std::unordered_map<UID, Actor*> World::actorUIDMap;
std::unordered_map<std::string, Actor*> World::actorNameMap;

std::vector<IActorSystem*> World::activeActorSystems;
std::vector<IComponentSystem*> World::activeComponentSystems;

void World::Init()
{
	//Add actorsystems into world
	for (auto actorSystem : ActorSystemCache::Get().GetAllSystems())
	{
		activeActorSystems.push_back(actorSystem);
	}

	//Add componentsystems into world
	for (auto& componentSystemIt : *componentSystemCache.typeToSystemMap)
	{
		activeComponentSystems.push_back(componentSystemIt.second);
	}

	//Load starting map
	FileSystem::LoadWorld(GameInstance::startingMap);
}

void World::Start()
{
	MaterialSystem::CreateAllMaterials();
	TextureSystem::CreateAllTextures();

	//Init actor systems
	for (IActorSystem* actorSystem : activeActorSystems)
	{
		actorSystem->Init();
	}

	//Init component systems
	for (IComponentSystem* componentSystem : activeComponentSystems)
	{
		componentSystem->Init();
	}

	if (Core::gameplayOn)
	{
		StartAllComponents();
		WakeAndStartAllActors();
	}
}

void World::WakeAndStartAllActors()
{
	auto actors = GetAllActorsInWorld();

	for (auto actor : actors)
	{
		actor->Awake();
	}

	for (auto actor : actors)
	{
		actor->Start();
	}

	for (auto actor : actors)
	{
		actor->LateStart();
	}
}

void World::StartAllComponents()
{
	auto components = GetAllComponentsInWorld();

	for (auto component : components)
	{
		component->Start();
	}
}

void World::EndAllActors()
{
	for (auto actor : GetAllActorsInWorld())
	{
		actor->End();
	}
}

void World::CreateDefaultMapActors()
{
	Player::system.Add();

	auto dlight = DirectionalLightActor::system.Add();
	//Set light pointing down because shadows looks nice.
	dlight->SetRotation(
		VMath::LookAtRotation(dlight->GetPositionV() - VMath::GlobalUpVector(), dlight->GetPositionV()));
	dlight->SetPosition(XMVectorSet(0.f, 5.f, 0.f, 1.f));

	auto grid = Grid::system.Add();
	grid->sizeX = 5;
	grid->sizeY = 5;

	//Set a ground plane to work off
	MeshActor::spawnMeshFilename = "node.fbx";
	auto mesh = MeshActor::system.Add();
	mesh->SetPosition(XMFLOAT3(2.f, -0.5f, 2.f));
	mesh->SetScale(XMVectorSet(5.f, 1.f, 5.f, 1.0f));
	MeshActor::spawnMeshFilename.clear();

	editor->UpdateWorldList();
}

std::vector<IActorSystem*> World::GetLayerActorSystems()
{
	std::vector<IActorSystem*> actorSystems;

	for (auto actorSystem : activeActorSystems)
	{
		if (actorSystem->GetName() == "EntranceTrigger" ||
			actorSystem->GetName() == "MeshActor")
		{
			continue;
		}

		actorSystems.push_back(actorSystem);
	}

	return actorSystems;
}

void World::TickAllActorSystems(float deltaTime)
{
	Profile::Start();

	for (IActorSystem* actorSystem : activeActorSystems)
	{
		actorSystem->Tick(deltaTime);
	}

	Profile::End();
}

void World::TickAllComponentSystems(float deltaTime)
{
	Profile::Start();

	for (IComponentSystem* componentSystem : activeComponentSystems)
	{
		componentSystem->Tick(deltaTime);
	}

	Profile::End();
}

std::vector<Actor*> World::GetAllActorsInWorld()
{
	std::vector<Actor*> outActors;

	for (IActorSystem* actorSystem : activeActorSystems)
	{
		auto actors = actorSystem->GetActorsAsBaseClass();
		outActors.insert(outActors.end(), actors.begin(), actors.end());
	}

	return outActors;
}

Actor* World::GetActorByUID(UID uid)
{
	auto actorIt = actorUIDMap.find(uid);
	assert(actorIt != actorUIDMap.end());
	return actorIt->second;
}

Actor* World::GetActorByUIDAllowNull(UID uid)
{
	auto actorIt = actorUIDMap.find(uid);
	if (actorIt == actorUIDMap.end())
	{
		return nullptr;
	}

	return actorIt->second;
}

Actor* World::GetActorByName(std::string actorName)
{
	auto actorIt = actorNameMap.find(actorName);
	assert(actorIt != actorNameMap.end());
	return actorIt->second;
}

Actor* World::GetActorByNameAllowNull(std::string actorName)
{
	Actor* foundActor = nullptr;
	foundActor = actorNameMap[actorName];
	if (foundActor == nullptr)
	{
		Log("%s actor not found.", actorName.c_str());
	}
	return foundActor;
}

std::vector<Component*> World::GetAllComponentsInWorld()
{
	std::vector<Component*> outComponents;

	for (IComponentSystem* componentSystem : activeComponentSystems)
	{
		auto components = componentSystem->GetComponentsAsBaseClass();
		outComponents.insert(outComponents.end(), components.begin(), components.end());
	}

	return outComponents;
}

void World::AddActorToWorld(Actor* actor)
{
	std::string actorName = actor->GetName();
	UID actorUID = actor->GetUID();

	assert(actorUIDMap.find(actorUID) == actorUIDMap.end());
	assert(actorNameMap.find(actorName) == actorNameMap.end());

	actorUIDMap.emplace(actorUID, actor);
	actorNameMap.emplace(actorName, actor);
}

void World::RemoveActorFromWorld(Actor* actor)
{
	actorUIDMap.erase(actor->GetUID());
	actorNameMap.erase(actor->GetName());
}

void World::RemoveActorFromWorld(UID actorUID)
{
	Actor* actor = actorUIDMap[actorUID];
	actorNameMap.erase(actor->GetName());
	actorUIDMap.erase(actorUID);
}

void World::RemoveActorFromWorld(std::string actorName)
{
	Actor* actor = actorNameMap[actorName];
	actorUIDMap.erase(actor->GetUID());
	actorNameMap.erase(actorName);
}

void World::ClearAllActorsFromWorld()
{
	actorUIDMap.clear();
	actorNameMap.clear();
}

bool World::CheckIfActorExistsInWorld(std::string actorName)
{
	return actorNameMap.find(actorName) != actorNameMap.end();
}

bool World::CheckIfActorExistsInWorld(UID actorUID)
{
	return actorUIDMap.find(actorUID) != actorUIDMap.end();
}

void World::Cleanup()
{
	actorUIDMap.clear();
	actorNameMap.clear();

	//Cleanup various systems
	Timer::Cleanup();
	PhysicsSystem::Reset();
	AudioSystem::DeleteLoadedAudioAndChannels();
	TextureSystem::Cleanup();
	MaterialSystem::Cleanup();
	SpriteSystem::Reset();
	UISystem::Reset();

	//CLEANUP COMPONENT SYSTEMS
	for (IComponentSystem* componentSystem : activeComponentSystems)
	{
		componentSystem->Cleanup();
	}

	//CLEANUP ACTOR SYSTEMS
	for (IActorSystem* actorSystem : activeActorSystems)
	{
		actorSystem->Cleanup();
	}
}
