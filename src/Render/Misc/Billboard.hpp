#pragma once

#include "Core/Common.hpp"
#include "Render/Cameras/Camera.hpp"
#include "Render/DX/RenderCommon.hpp"

class CBillboard
{
public:
    CBillboard(ID3D11DeviceContext* context, std::wstring tex, Vector3 pos, float scale, Color tint);
    ~CBillboard();

    void Render(const ICamera& cam);
    void SetPosition(const Vector3& pos) { Position = pos; }
    void SetScale(float scale) { Scale = scale; }
    void SetTint(const Color& tint) { Tint = tint; PixelCB->SetData(Context, Tint); }

private:
    struct Vertex
    {
        Vector3 Position;
        Vector2 UV;
    };

    struct VSBuffer
    {
        Matrix ViewProj;
        float Scale;
        Vector3 _Pad;
    };

    float Scale;
    Vector3 Position;
    Matrix World;
    Color Tint;

    ID3D11DeviceContext* Context;

    RenderPipeline Pipeline;
    ComPtr<ID3D11Buffer> VertexBuffer;
    ComPtr<ID3D11Buffer> IndexBuffer;
    ComPtr<ID3D11ShaderResourceView> Texture;

    std::unique_ptr<ConstantBuffer<VSBuffer>> VertexCB;
    std::unique_ptr<ConstantBuffer<Color>> PixelCB;
};