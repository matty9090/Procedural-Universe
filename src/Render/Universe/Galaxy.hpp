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

#ifdef _DEBUG
    #define PARTICLES_PER_GALAXY  100000
#else
    #define PARTICLES_PER_GALAXY 500000
#endif

class Galaxy
{
public:
    Galaxy(ID3D11DeviceContext* context, bool onlyRenderDust = false);

    static void LoadCache(ID3D11Device* device, ID3D11DeviceContext* context);

    void InitialSeed(uint64_t seed);
    void FinishSeed(const std::vector<LWParticle>& particles);

    void Move(DirectX::SimpleMath::Vector3 v);
    void Scale(float scale);
    void SetFades(bool fade) { DustRenderer->SetFades(fade); }

    void Render(const ICamera& cam, float t, float scale = 1.0f, DirectX::SimpleMath::Vector3 voffset = DirectX::SimpleMath::Vector3::Zero, bool single = false);
    void RenderImposter(const ICamera& cam, float scale = 1.0f);

    DirectX::SimpleMath::Color GetColour() const { return Colour; }
    DirectX::SimpleMath::Vector3 GetPosition() const { return Position; }
    DirectX::SimpleMath::Vector3 GetClosestObject(DirectX::SimpleMath::Vector3 pos);
    
    size_t GetClosestObjectIndex() const { return CurrentClosestObjectID; }
    uint64_t GetSeed() const { return Seed; }
    LWParticle GetParticle(size_t index) const { return Particles[index]; }

    std::string Name;

    static float ImposterThreshold;
    static float ImposterFadeDist;
    static float ImposterOffsetPercent;

    static unsigned int NumDustClouds;

private:
    void RegenerateBuffer();

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;

    static RenderPipeline ParticlePipeline;

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

    bool OnlyRenderDust = false;
    float ImposterSize = 250.0f;
    Color Colour;
    size_t CurrentClosestObjectID = 0;
    uint64_t Seed = 0U;
    DirectX::SimpleMath::Vector3 Position;

    // Only one galaxy should render stars at any time
    static Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;

    std::vector<LWParticle> Particles;
    std::vector<BillboardInstance> DustClouds;
    std::unique_ptr<CBillboard> DustRenderer;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> StarTexture;

    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};