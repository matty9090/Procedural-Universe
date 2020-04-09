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
#include <SpriteFont.h>
#include <SpriteBatch.h>
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
    enum EObjectType { Galaxy, Star, Planet };

    void Clear();
    void RenderUI();
    void FloatingOrigin();
    void TransitionLogic();
    void CreateModelPipeline();
    void SetupTargets();
    void Travel(EObjectType type);
    void TravelUpdate(float dt);
    std::string GetSpeedStr(double speed);

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    DirectX::Mouse* Mouse;
    DirectX::Keyboard* Keyboard;
    DX::DeviceResources* DeviceResources;
    DirectX::Keyboard::KeyboardStateTracker Tracker;

    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::SpriteFont> Font;
    std::unique_ptr<DirectX::SpriteBatch> SpriteBatch;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

    std::unique_ptr<Cube>             ClosestObjCube;
    std::unique_ptr<CSandboxCamera>   Camera;
    std::unique_ptr<SandboxTarget>    RootTarget;

    SandboxTarget* CurrentTarget;
    RenderPipeline ModelPipeline;

    enum ETravelState { Panning, Travelling };

    float PanSpeed = 0.4f;
    float TravelSpeed = 0.4f;
    float TravelT = 0.0f;
    float TravelStopDist = 210.0f;
    bool IsTravelling = false;
    EObjectType TravelType, CurrentTravelType;
    ETravelState TravelState;
    Vector3 TravelTarget;
    Quaternion TravelRotStart, TravelRotEnd;

    bool FreezeTransitions = false;
    float CamOriginSnapThreshold = 5000.0f;
    float CurrentTransitionT = 0.0f;
    int Frames = 0;
    float FrameTimer = 0.0f;

    bool bShowClosestObject = false;
    size_t ClosestObjIndex;
};