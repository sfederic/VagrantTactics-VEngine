#include "vpch.h"
#include "ShaderSystem.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Input.h"
#include "Log.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ShaderItem.h"
#include "Profile.h"
#include "ShaderItem.h"

void CompileAllShadersFromFile();
void HotreloadShaders();
void RecompileShaderTypesForHotreload(const std::string shaderType, const std::string version);

std::map<std::wstring, std::unique_ptr<VertexShader>> vertexShaders;
std::map<std::wstring, std::unique_ptr<PixelShader>> pixelShaders;
std::map<std::string, std::shared_ptr<ShaderItem>> shaderItems;

void ShaderSystem::Init()
{
    CompileAllShadersFromFile();

    ShaderItems::Default = new ShaderItem("Default", L"Default_vs.cso", L"Default_ps.cso");
    ShaderItems::DefaultClip = new ShaderItem("DefaultClip", L"Default_vs.cso", L"TextureClip_ps.cso");
    ShaderItems::Unlit = new ShaderItem("Unlit", L"Default_vs.cso", L"TextureClip_ps.cso");
    ShaderItems::Animation = new ShaderItem("Animation", L"Animation_vs.cso", L"Default_ps.cso");
    ShaderItems::Shadow = new ShaderItem("Shadow", L"Shadows_vs.cso", L"Shadows_ps.cso");
    ShaderItems::Instance = new ShaderItem("Instance", L"Instance_vs.cso", L"Instance_ps.cso");
    ShaderItems::SolidColour = new ShaderItem("SolidColour", L"Default_vs.cso", L"SolidColour_ps.cso");
    ShaderItems::UI = new ShaderItem("UI", L"UI_vs.cso", L"TextureClip_ps.cso");
    ShaderItems::PostProcess = new ShaderItem("PostProcess", L"PostProcess_vs.cso", L"PostProcess_ps.cso");
    ShaderItems::Astral = new ShaderItem("Astral", L"Default_vs.cso", L"SolidColour_ps.cso");
}

void ShaderSystem::Tick()
{
    HotreloadShaders();
}

VertexShader* ShaderSystem::FindVertexShader(const std::wstring filename)
{
    assert(vertexShaders.find(filename) != vertexShaders.end());
    return vertexShaders[filename].get();
}

PixelShader* ShaderSystem::FindPixelShader(const std::wstring filename)
{
    assert(pixelShaders.find(filename) != pixelShaders.end());
    return pixelShaders[filename].get();
}

void ShaderSystem::AddShaderItem(ShaderItem* shaderItem)
{
    shaderItems.emplace(shaderItem->GetName(), shaderItem);
}

ShaderItem* ShaderSystem::FindShaderItem(const std::string shaderItemName)
{
    assert(shaderItems.find(shaderItemName) != shaderItems.end());
    return shaderItems[shaderItemName].get();
}

void ShaderSystem::ClearShaders()
{
    vertexShaders.clear();
    pixelShaders.clear();
}

void CompileAllShadersFromFile()
{
    ShaderSystem::ClearShaders();

    for (auto& entry : std::filesystem::directory_iterator("Shaders/Vertex"))
    {
        auto vertexShader = std::make_unique<VertexShader>();
        vertexShader->Create(entry.path().c_str());
        vertexShaders.emplace(entry.path().filename(), std::move(vertexShader));
    }

    for (auto& entry : std::filesystem::directory_iterator("Shaders/Pixel"))
    {
        auto pixelShader = std::make_unique<PixelShader>();
        pixelShader->Create(entry.path().c_str());
        pixelShaders.emplace(entry.path().filename(), std::move(pixelShader));
    }
}

void HotreloadShaders()
{
    if (Input::GetKeyUp(Keys::F4))
    {
        auto startTime = Profile::QuickStart();

        RecompileShaderTypesForHotreload("Vertex", "vs_5_0");
        RecompileShaderTypesForHotreload("Pixel", "ps_5_0");

        CompileAllShadersFromFile();

        Log("%d vertex shaders recompiled.", vertexShaders.size());
        Log("%d pixel shaders recompiled.", vertexShaders.size());

        double endTime = Profile::QuickEnd(startTime);
        Log("Shader hotreload took %f seconds.", endTime);
    }
}

void RecompileShaderTypesForHotreload(const std::string shaderType, const std::string version)
{
    for (const auto& entry : std::filesystem::directory_iterator("Code/Render/Shaders/" + shaderType + "/"))
    {
        auto filename = entry.path().filename();
        std::string outputFilepath = "Shaders/" + shaderType + "/";

        std::string outputFile = outputFilepath + filename.replace_extension(".cso").string();

        std::string command = "Tools/fxc /Od /Zi /T " + version + " /Fo " + outputFile + " Code/Render/Shaders/" + shaderType + "/" + entry.path().filename().string();
        std::system(command.c_str());
    }
}
