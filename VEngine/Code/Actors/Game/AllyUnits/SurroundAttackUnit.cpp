#include "vpch.h"
#include "SurroundAttackUnit.h"
#include "Components/MeshComponent.h"
#include "Actors/Game/Grid.h"
#include "Actors/Game/Unit.h"
#include "Gameplay/GameUtils.h"

SurroundAttackUnit::SurroundAttackUnit()
{
	mesh = CreateComponent(MeshComponent("bevel_rock.fbx", "test.png"), "Mesh");
	rootComponent->AddChild(mesh);
}

void SurroundAttackUnit::AttackPattern()
{
	auto grid = Grid::system.GetFirstActor();

	auto nodes = grid->GetNeighbouringActiveAndInactiveNodesForceful(GetCurrentNode());
	for (auto node : nodes)
	{
		GameUtils::SpawnSpriteSheet("Sprites/explosion.png", node->worldPosition, false, 4, 4);
	}

	for (auto unit : grid->GetAllUnitsFromNodes(nodes))
	{
		unit->InflictDamage(attackPoints);
	}
}
