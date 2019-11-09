#include "ResourceManager.hpp"
#include "Services/Log.hpp"

#include "Render/DX/Shader.hpp"
#include "Render/Model/Mesh.hpp"

#include <WICTextureLoader.h>

ID3D11VertexShader* FResourceManager::GetVertexShader(std::wstring file)
{
    if (IsCachedComResource(file))
        return reinterpret_cast<ID3D11VertexShader*>(ComResourceCache[file].Ptr);

    ID3D11VertexShader** shader = reinterpret_cast<ID3D11VertexShader**>(&ComResourceCache[file].Ptr);
    ID3DBlob** code = reinterpret_cast<ID3DBlob**>(&ComResourceCache[file + L"blob"].Ptr);

    LoadVertexShader(Device, file, shader, code);

    return *shader;
}

ID3DBlob* FResourceManager::GetVertexCode(std::wstring file)
{
    if (IsCachedComResource(file))
        return reinterpret_cast<ID3DBlob*>(ComResourceCache[file + L"blob"].Ptr);

    LOGE("Tried to get vertex code before vertex shader was loaded!");

    return nullptr;
}

ID3D11PixelShader* FResourceManager::GetPixelShader(std::wstring file)
{
    if (IsCachedComResource(file))
        return reinterpret_cast<ID3D11PixelShader*>(ComResourceCache[file].Ptr);

    ID3D11PixelShader** shader = reinterpret_cast<ID3D11PixelShader**>(&ComResourceCache[file].Ptr);
    LoadPixelShader(Device, file, shader);

    return *shader;
}

ID3D11GeometryShader* FResourceManager::GetGeometryShader(std::wstring file)
{
    if (IsCachedComResource(file))
        return reinterpret_cast<ID3D11GeometryShader*>(ComResourceCache[file].Ptr);

    ID3D11GeometryShader** shader = reinterpret_cast<ID3D11GeometryShader**>(&ComResourceCache[file].Ptr);
    LoadGeometryShader(Device, file, shader);

    return *shader;
}

ID3D11ShaderResourceView* FResourceManager::GetTexture(std::wstring file)
{
    if (IsCachedComResource(file))
        return reinterpret_cast<ID3D11ShaderResourceView*>(ComResourceCache[file].Ptr);

    ID3D11ShaderResourceView** texture = reinterpret_cast<ID3D11ShaderResourceView**>(&ComResourceCache[file].Ptr);
    DirectX::CreateWICTextureFromFile(Device, file.c_str(), nullptr, texture);

    LOGM("Loaded texture " + wstrtostr(file))

    return *texture;
}

CMesh* FResourceManager::GetMesh(std::string mesh)
{
    if (IsCachedMesh(mesh)) return reinterpret_cast<CMesh*>(ResourceCache[mesh].Ptr);

    CMesh** m = reinterpret_cast<CMesh**>(&ResourceCache[mesh].Ptr);
    *m = CMesh::Load(Device, mesh).release();

    return *m;
}

bool FResourceManager::IsCachedMesh(std::string res)
{
    bool r = ResourceCache.find(res) != ResourceCache.end();
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
    for (auto& resource : ComResourceCache)
    {
        ((IUnknown*)resource.second.Ptr)->Release();
    }

    for (auto& resource : ResourceCache)
    {
        delete resource.second.Ptr;
    }
}
