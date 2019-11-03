#include "ResourceManager.hpp"
#include "Services/Log.hpp"

#include "Render/Shader.hpp"
#include "Render/Mesh.hpp"

#include <WICTextureLoader.h>

ID3D11VertexShader* FResourceManager::GetVertexShader(std::wstring file)
{
    if (IsCachedComResource(file)) return ComResourceCache[file].Vertex;

    ID3D11VertexShader** shader = &ComResourceCache[file].Vertex;
    ID3DBlob** code = &ComResourceCache[file + L"blob"].ShaderCode;

    LoadVertexShader(Device, file, shader, code);

    return *shader;
}

ID3DBlob* FResourceManager::GetVertexCode(std::wstring file)
{
    if (IsCachedComResource(file + L"blob"))
        return ComResourceCache[file + L"blob"].ShaderCode;

    LOGE("Tried to get vertex code before vertex shader was loaded!");

    return nullptr;
}

ID3D11PixelShader* FResourceManager::GetPixelShader(std::wstring file)
{
    if (IsCachedComResource(file)) return ComResourceCache[file].Pixel;

    ID3D11PixelShader** shader = &ComResourceCache[file].Pixel;
    LoadPixelShader(Device, file, shader);

    return *shader;
}

ID3D11GeometryShader* FResourceManager::GetGeometryShader(std::wstring file)
{
    if (IsCachedComResource(file)) return ComResourceCache[file].Geometry;

    ID3D11GeometryShader** shader = &ComResourceCache[file].Geometry;
    LoadGeometryShader(Device, file, shader);

    return *shader;
}

ID3D11ShaderResourceView* FResourceManager::GetTexture(std::wstring file)
{
    if (IsCachedComResource(file)) return ComResourceCache[file].Texture;

    ID3D11ShaderResourceView** texture = &ComResourceCache[file].Texture;
    DirectX::CreateWICTextureFromFile(Device, file.c_str(), nullptr, texture);

    LOGM("Loaded texture " + wstrtostr(file))

    return *texture;
}

CMesh* FResourceManager::GetMesh(std::string mesh)
{
    if (IsCachedMesh(mesh)) return MeshCache[mesh];

    CMesh** m = &MeshCache[mesh];
    *m = CMesh::Load(Device, mesh).release();

    return *m;
}

bool FResourceManager::IsCachedMesh(std::string res)
{
    bool r = MeshCache.find(res) != MeshCache.end();
    if (r) LOGV("Found " + res + " in cache");
    return r;
}

bool FResourceManager::IsCachedComResource(std::wstring res)
{
    bool r = ComResourceCache.find(res) != ComResourceCache.end();
    if (r) LOGV("Found " + wstrtostr(res) + " in cache");
    return r;
}

FResourceManager::~FResourceManager()
{

}
