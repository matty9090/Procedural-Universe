#include "Mesh.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

#include <WICTextureLoader.h>

CMesh::CMesh(ID3D11Device* device, std::vector<MeshVertex> vertices, std::vector<unsigned int> indices, ID3D11ShaderResourceView* texture)
    : NumIndices(static_cast<unsigned int>(indices.size())), Texture(texture)
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(MeshVertex) * static_cast<UINT>(vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &vertices[0];

    DX::ThrowIfFailed(device->CreateBuffer(&vbd, &initData, VertexBuffer.ReleaseAndGetAddressOf()));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
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
        LOGM("Failed to load mesh " + file)
        return nullptr;
    }

    std::function<CMesh*(aiMesh*, const aiScene*)> ProcessMesh = [&](aiMesh* mesh, const aiScene* scene) {
        std::vector<MeshVertex> vertices;
        std::vector<unsigned int> indices;
        ID3D11ShaderResourceView* texture = nullptr;
        
        DirectX::SimpleMath::Vector3 mmin = { 9999, 9999, 9999};
        DirectX::SimpleMath::Vector3 mmax = { -9999, -9999, -9999 };

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            MeshVertex v;
            v.Position.x = mesh->mVertices[i].x;
            v.Position.y = mesh->mVertices[i].y;
            v.Position.z = mesh->mVertices[i].z;

            if (v.Position.x < mmin.x) mmin.x = v.Position.x;
            if (v.Position.y < mmin.y) mmin.y = v.Position.y;
            if (v.Position.z < mmin.z) mmin.z = v.Position.z;
            if (v.Position.x > mmax.x) mmax.x = v.Position.x;
            if (v.Position.y > mmax.y) mmax.y = v.Position.y;
            if (v.Position.z > mmax.z) mmax.z = v.Position.z;

            if (mesh->mTextureCoords[0])
            {
                v.TexCoord.x = mesh->mTextureCoords[0][i].x;
                v.TexCoord.y = mesh->mTextureCoords[0][i].y;
            }

            vertices.push_back(v);
        }

        LOGM(mmin)
        LOGM(mmax)

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

                std::string file = std::string("assets/") + str.C_Str();
                std::wstring filew(file.begin(), file.end());

                texture = RESM.GetTexture(filew.c_str());
            }
        }

        return new CMesh(device, vertices, indices, texture);
    };

    CMesh* root = ProcessMesh(scene->mMeshes[0], scene);

    LOGM("Loaded mesh " + file)

    return std::unique_ptr<CMesh>(root);
}