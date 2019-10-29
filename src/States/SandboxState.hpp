#pragma once

#include "Core/State.hpp"

#include "Render/Camera.hpp"
#include "Render/Particle.hpp"
#include "Render/ConstantBuffer.hpp"

#include <list>
#include <vector>
#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <CommonStates.h>

#include "Render/Model.hpp"

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
    DirectX::Mouse* Mouse;
    RenderPipeline ModelPipeline;
    DX::DeviceResources* DeviceResources;

    std::unique_ptr<CModel>            Ship;
    std::unique_ptr<CCamera>           Camera;
    std::list<std::unique_ptr<CMesh>>  Meshes;

    enum class EState
    {
        
    };
};