#pragma once

#include "Core/State.hpp"

#include "Render/ShipCamera.hpp"
#include "Render/Particle.hpp"
#include "Render/ConstantBuffer.hpp"

#include <list>
#include <vector>
#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <PostProcess.h>
#include <CommonStates.h>

#include "Render/Ship.hpp"

struct SandboxStateData : public StateData
{
    std::vector<Particle> Particles;

    SandboxStateData(std::vector<Particle> particles) : Particles(particles) {}
};

class SandboxState : public IState
{
public:
    // IState override
    void Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data) override;
    void Cleanup() override;
    void Update(float dt) override;
    void Render() override;
    // End IState override

private:
    void Clear();
    void CreateModelPipeline();
    void CreateParticlePipeline();

    struct GSConstantBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
    };

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    DirectX::Mouse* Mouse;
    DirectX::Keyboard* Keyboard;
    DX::DeviceResources* DeviceResources;
    
    RenderView ParticleRenderTarget;
    RenderPipeline ModelPipeline;
    RenderPipeline ParticlePipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<DirectX::DualPostProcess> DualPostProcess;
    std::unique_ptr<DirectX::BasicPostProcess> BasicPostProcess;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;
    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;

    std::unique_ptr<CShip>            Ship;
    std::unique_ptr<CShipCamera>      Camera;
    std::list<std::unique_ptr<CMesh>> Meshes;

    enum class EState
    {
        
    };
};