#pragma once

#include "Core/State.hpp"
#include "SandboxTarget.hpp"

#include "Render/ShipCamera.hpp"
#include "Render/Particle.hpp"
#include "Render/PostProcess.hpp"
#include "Render/ConstantBuffer.hpp"
#include "Render/Ship.hpp"

#include <list>
#include <vector>
#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <PostProcess.h>
#include <CommonStates.h>


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
    void SetupTargets(const std::vector<Particle>& seedData);

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    DirectX::Mouse* Mouse;
    DirectX::Keyboard* Keyboard;
    DX::DeviceResources* DeviceResources;

    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

    std::unique_ptr<CShip>            Ship;
    std::unique_ptr<CShipCamera>      Camera;
    std::list<std::unique_ptr<CMesh>> Meshes;

    std::unique_ptr<SandboxTarget> RootTarget;
    SandboxTarget* CurrentTarget;
    RenderPipeline ModelPipeline;

    float CamLengthThreshold = 5000.0f;
};