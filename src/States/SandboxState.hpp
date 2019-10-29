#pragma once

#include "Core/State.hpp"

#include "Render/Camera.hpp"
#include "Render/Particle.hpp"
#include "Render/ConstantBuffer.hpp"

#include <vector>
#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
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

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    DX::DeviceResources* DeviceResources;
};