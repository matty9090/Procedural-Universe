#pragma once

#include <map>
#include <string>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>

#define RESM FResourceManager::Get()

class CMesh;

class FResourceManager
{
public:
    static FResourceManager& Get()
    {
        static FResourceManager instance;
        return instance;
    }

    FResourceManager(FResourceManager const&) = delete;
    void operator=(FResourceManager const&) = delete;
    ~FResourceManager();

    void SetDevice(ID3D11Device* device) { Device = device; }

    ID3D11VertexShader*       GetVertexShader(std::wstring file);
    ID3DBlob*                 GetVertexCode(std::wstring file);
    ID3D11PixelShader*        GetPixelShader(std::wstring file);
    ID3D11GeometryShader*     GetGeometryShader(std::wstring file);
    ID3D11ShaderResourceView* GetTexture(std::wstring file);
    
    CMesh*                    GetMesh(std::string mesh);

private:
    FResourceManager() {}

    ID3D11Device* Device = nullptr;

    bool IsCachedMesh(std::string res);
    bool IsCachedComResource(std::wstring res);

    union ComResource
    {
        ID3D11VertexShader* Vertex;
        ID3D11PixelShader* Pixel;
        ID3D11GeometryShader* Geometry;
        ID3D11ShaderResourceView* Texture;
        ID3D11RasterizerState* Raster;
        ID3DBlob* ShaderCode;
    };

    struct ComCacheEntry
    {
        void* Ptr;

        enum EType
        {
            Vertex,
            Pixel,
            Geometry,
            Texture,
            Raster,
            ShaderCode
        } Type;
    };

    struct CacheEntry
    {
        void* Ptr;

        enum EType
        {
            Mesh
        } Type;
    };

    std::map<std::string , CacheEntry>    ResourceCache;
    std::map<std::wstring, ComCacheEntry> ComResourceCache;
};