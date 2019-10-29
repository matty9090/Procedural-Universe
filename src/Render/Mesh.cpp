#include "Mesh.hpp"
#include "Services/Log.hpp"

#include <WICTextureLoader.h>

CMesh::CMesh(ID3D11Device* device, std::vector<MeshVertex> vertices, std::vector<unsigned int> indices, ID3D11ShaderResourceView* texture)
    : NumIndices(indices.size()), Texture(texture)
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(MeshVertex) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &vertices[0];

    DX::ThrowIfFailed(device->CreateBuffer(&vbd, &initData, VertexBuffer.ReleaseAndGetAddressOf()));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    initData.pSysMem = &indices[0];

    DX::ThrowIfFailed(device->CreateBuffer(&ibd, &initData, IndexBuffer.ReleaseAndGetAddressOf()));
}

std::unique_ptr<CMesh> CMesh::Load(ID3D11Device* device, std::string file)
{
    Assimp::Importer importer;

    auto scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!scene)
    {
        FLog::Get().Log("Failed to load mesh " + file);
        return nullptr;
    }

    std::function<CMesh*(aiMesh*, const aiScene*)> ProcessMesh = [&](aiMesh* mesh, const aiScene* scene) {
        std::vector<MeshVertex> vertices;
        std::vector<unsigned int> indices;
        ID3D11ShaderResourceView* texture = nullptr;

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            MeshVertex v;
            v.Position.x = mesh->mVertices[i].x;
            v.Position.y = mesh->mVertices[i].y;
            v.Position.z = mesh->mVertices[i].z;

            if (mesh->mTextureCoords[0])
            {
                v.TexCoord.x = mesh->mTextureCoords[0][i].x;
                v.TexCoord.y = mesh->mTextureCoords[0][i].y;
            }

            vertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            auto face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0)
        {
            auto material = scene->mMaterials[mesh->mMaterialIndex];
            
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

                std::string file = std::string("resources/") + str.C_Str();
                std::wstring filew(file.begin(), file.end());

                DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(device, filew.c_str(), nullptr, &texture));

                FLog::Get().Log("Loaded texture " + file);
            }
        }

        return new CMesh(device, vertices, indices, texture);
    };

    CMesh* root = ProcessMesh(scene->mMeshes[0], scene);

    FLog::Get().Log("Loaded mesh " + file);

    return std::unique_ptr<CMesh>(root);
}