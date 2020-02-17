#pragma once

#include "Core/Common.hpp"
#include "Render/Cameras/Camera.hpp"
#include "Render/DX/RenderCommon.hpp"
#include "Render/DX/ConstantBuffer.hpp"

struct BillboardInstance
{
    Vector3 Position;
    float Scale;
    Color Tint;
};

class CBillboard
{
public:
    CBillboard(ID3D11DeviceContext* context, std::wstring tex, bool fades = false, unsigned int reserve = 1, std::vector<BillboardInstance> instances = {});
    ~CBillboard();

    void Render(const ICamera& cam);
    void Render(const ICamera& cam, float scale, Vector3 offset);

    void Scale(float s) { RelativeScale *= s; }
    void SetFades(bool fade) { Fades = fade; }
    void SetPosition(Vector3 pos);
    void UpdateInstances(std::vector<BillboardInstance> instances);

private:
    struct VSBuffer
    {
        Matrix ViewProj;
        Matrix InvView;
        Vector3 Cam;
        float Fades;
    };

    bool Fades = false;
    float RelativeScale = 1.0f;
    Vector3 Position = Vector3::Zero;

    ID3D11DeviceContext* Context;
    std::vector<BillboardInstance> Instances;

    RenderPipeline Pipeline;
    Microsoft::WRL::ComPtr<ID3D11Buffer> InstanceBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture;

    std::unique_ptr<ConstantBuffer<VSBuffer>> VertexCB;
};