#pragma once

#include <vector>
#include <memory>

#include <d3d11.h>
#include <SimpleMath.h>
#include <CommonStates.h>

#include "Core/Common.hpp"

#include "Render/Cameras/Camera.hpp"
#include "Render/Misc/Particle.hpp"
#include "Render/Misc/Billboard.hpp"
#include "Render/DX/RenderCommon.hpp"
#include "Render/DX/ConstantBuffer.hpp"

class Galaxy
{
public:
    Galaxy(ID3D11DeviceContext* context);

    void Seed(uint64_t seed);
    void Move(DirectX::SimpleMath::Vector3 v);
    void Scale(float scale);
    void Render(const ICamera& cam, float t, float scale = 1.0f, DirectX::SimpleMath::Vector3 voffset = DirectX::SimpleMath::Vector3::Zero, bool single = false);

    DirectX::SimpleMath::Vector3 GetPosition() const { return Position; }
    DirectX::SimpleMath::Vector3 GetClosestObject(DirectX::SimpleMath::Vector3 pos);
    size_t GetClosestObjectIndex() const { return CurrentClosestObjectID; }

    static float ImposterThreshold;
    static float ImposterFadeDist;
    static float ImposterOffsetPercent;

private:
    void RegenerateBuffer();

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    RenderPipeline ParticlePipeline;

    struct GSConstantBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
        DirectX::SimpleMath::Vector3 Translation;
        float Custom;
    };

    struct LerpConstantBuffer
    {
        float Alpha;
        float Custom1, Custom2, Custom3;
    };

    Color Colour;
    size_t CurrentClosestObjectID = 0;
    DirectX::SimpleMath::Vector3 Position;

    std::vector<Particle> Particles;
    std::unique_ptr<CBillboard> Imposter;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;

    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};