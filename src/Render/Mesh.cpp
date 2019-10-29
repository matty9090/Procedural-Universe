#include "Mesh.hpp"
#include "Services/Log.hpp"

Mesh::Mesh(ID3D11Device* device, std::vector<MeshVertex> vertices, std::vector<unsigned int> indices) : Device(device)
{
    
}

std::unique_ptr<Mesh> Mesh::Load(ID3D11Device* device, std::string file)
{
    Assimp::Importer importer;

    auto scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!scene)
    {
        FLog::Get().Log("Failed to load mesh " + file);
        return nullptr;
    }

    std::function<Mesh*(aiMesh*, const aiScene*)> ProcessMesh = [&](aiMesh* mesh, const aiScene* scene) {
        std::vector<MeshVertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<ID3D11ShaderResourceView*> textures;

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

        return new Mesh(device, vertices, indices);
    };

    Mesh* root = ProcessMesh(scene->mMeshes[0], scene);

    FLog::Get().Log("Loaded mesh " + file);

    return std::unique_ptr<Mesh>(root);
}