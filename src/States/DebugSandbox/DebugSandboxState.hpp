#pragma once

#include "Core/State.hpp"
#include "States/Sandbox/SandboxTarget.hpp"

#include "Render/Cameras/ShipCamera.hpp"
#include "Render/Misc/Particle.hpp"
#include "Render/DX/ConstantBuffer.hpp"
#include "Render/Model/Ship.hpp"

#include <list>
#include <vector>
#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <PostProcess.h>
#include <CommonStates.h>


struct DebugSandboxStateData : public StateData
{
    std::vector<Particle> Particles;

    DebugSandboxStateData(std::vector<Particle> particles) : Particles(particles) {}
};

class DebugSandboxState : public IState
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
    void RenderUI();
    void SetupTargets(const std::vector<Particle>& seedData);
    void TestTransitions();

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    DirectX::Mouse* Mouse;
    DirectX::Keyboard* Keyboard;
    DX::DeviceResources* DeviceResources;

    std::unique_ptr<CShipCamera> Camera;

    std::unique_ptr<SandboxTarget> RootTarget;
    SandboxTarget* CurrentTarget;
};