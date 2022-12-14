#include "vpch.h"
#include "DestructibleMeshComponent.h"
#include "Asset/FBXLoader.h"
#include "Render/Material.h"
#include "Render/RenderUtils.h"

DestructibleMeshComponent::DestructibleMeshComponent(const std::string filename_,
    const std::string textureFilename_,
    ShaderItem* shaderItem)
    : MeshComponent(filename_, textureFilename_, shaderItem)
{
}

void DestructibleMeshComponent::Create()
{
	meshComponentData.meshComponent = this;

	material->Create();

	FBXLoader::ImportFracturedMesh(meshComponentData.filename.c_str(), meshDatas);

	int meshIndex = 0;

	for (auto& meshData : meshDatas)
	{
		//@Todo: because the mesh components here have no actor parent, their deserialisation
		//on world load will crash over owner UIDs not being found.
		auto mesh = MeshComponent::system.Add("TempDestructibleMesh" + std::to_string(meshIndex));

		//parent all the fractured cell meshes to this to be able to move it around before breaking.
		this->AddChild(mesh);

		meshCells.push_back(mesh);

		mesh->isStatic = false;

		mesh->meshDataProxy.vertices = &meshData.vertices;
		mesh->meshDataProxy.indices = &meshData.indices;

		//Setup bounds
		BoundingOrientedBox::CreateFromPoints(mesh->boundingBox, mesh->meshDataProxy.vertices->size(),
			&mesh->meshDataProxy.vertices->at(0).pos, sizeof(Vertex));

		mesh->pso.vertexBuffer = new Buffer();
		mesh->pso.indexBuffer = new Buffer();
		mesh->pso.vertexBuffer->data = RenderUtils::CreateVertexBuffer(mesh->meshDataProxy);
		mesh->pso.indexBuffer->data = RenderUtils::CreateIndexBuffer(mesh->meshDataProxy);

		meshIndex++;
	}
}

Properties DestructibleMeshComponent::GetProps()
{
    auto props = __super::GetProps();
    props.title = "DestructibleMeshComponent";
    return props;
}
