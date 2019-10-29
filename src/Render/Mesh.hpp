#pragma once

#include <string>
#include <vector>
#include <memory>
#include <d3d11.h>
#include <SimpleMath.h>

#include <assimp/scene.h>
#include <assimp/importer.hpp>
#include <assimp/postprocess.h>

#include "Render/RenderCommon.hpp"

struct MeshVertex
{
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Vector2 TexCoord;
};

class Mesh
{
public:
    Mesh(ID3D11Device* device, std::vector<MeshVertex> vertices, std::vector<unsigned int> indices);

    static std::unique_ptr<Mesh> Load(ID3D11Device* device, std::string file);

    std::vector<std::unique_ptr<Mesh>> Meshes;

private:
    RenderPipeline Pipeline;

    ID3D11Device* Device;
    ID3D11Buffer* VertexBuffer = nullptr;
    ID3D11Buffer* IndexBuffer = nullptr;

    std::unique_ptr<Mesh> Children;
};