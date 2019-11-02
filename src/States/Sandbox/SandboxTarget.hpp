#pragma once

#include "App/DeviceResources.hpp"
#include "Core/Maths.hpp"

#include "Render/ShipCamera.hpp"
#include "Render/ConstantBuffer.hpp"
#include "Render/PostProcess.hpp"
#include "Render/RenderCommon.hpp"

#include <string>
#include <memory>

#include <d3d11.h>
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;

class SandboxTarget
{
public:
    SandboxTarget(ID3D11DeviceContext* context, std::string name, DX::DeviceResources* resources, CShipCamera* camera);

    void Update(float dt);
    bool IsTransitioning() const { return State == EState::Transitioning; }

    virtual void Render() = 0;
    virtual void MoveObjects(Vector3 v) {}
    virtual Vector3 GetClosestObject(Vector3 pos) const = 0;

    void BeginTransition();
    void EndTransition();

    std::string Name;
    float Scale = 1.0f;
    SandboxTarget* Parent = nullptr;
    std::unique_ptr<SandboxTarget> Child = nullptr;

    float BeginTransitionDist = 1000.0f;
    float EndTransitionDist = 400.0f;

protected:
    virtual void StateIdle() {}
    virtual void StateTransitioning() {}

    virtual void OnBeginTransition() {}
    virtual void OnEndTransition() {}

    CShipCamera* Camera = nullptr;
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* Context = nullptr;
    DX::DeviceResources* Resources = nullptr;

private:
    enum class EState
    {
        Idle,
        Transitioning
    } State;
};