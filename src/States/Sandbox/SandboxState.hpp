#pragma once

#include "Core/State.hpp"
#include "SandboxTarget.hpp"

#include "Render/Cameras/SandboxCamera.hpp"
#include "Render/Misc/Particle.hpp"
#include "Render/Misc/PostProcess.hpp"
#include "Render/DX/ConstantBuffer.hpp"
#include "Render/Model/Cube.hpp"
#include "Render/Planet/Planet.hpp"

#include <list>
#include <vector>
#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <PostProcess.h>
#include <CommonStates.h>

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
    void FloatingOrigin();
    void TransitionLogic();
    void CreateModelPipeline();
    void SetupTargets();

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    DirectX::Mouse* Mouse;
    DirectX::Keyboard* Keyboard;
    DX::DeviceResources* DeviceResources;
    DirectX::Keyboard::KeyboardStateTracker Tracker;

    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

    std::unique_ptr<Cube>             ClosestObjCube;
    std::unique_ptr<CPlanet>          Planet;
    std::unique_ptr<CSandboxCamera>   Camera;
    std::unique_ptr<SandboxTarget>    RootTarget;

    SandboxTarget* CurrentTarget;
    RenderPipeline ModelPipeline;

    float CamOriginSnapThreshold = 5000.0f;
    float CurrentTransitionT = 0.0f;
    int Frames = 0;
    float FrameTimer = 0.0f;

    bool bShowClosestObject = false;
    size_t ClosestObjIndex;
};