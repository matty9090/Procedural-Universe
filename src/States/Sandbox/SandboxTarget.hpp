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
#include "Services/Log.hpp"
using DirectX::SimpleMath::Vector3;

class SandboxTarget
{
public:
    SandboxTarget(ID3D11DeviceContext* context, std::string name, DX::DeviceResources* resources, CShipCamera* camera);
    virtual ~SandboxTarget() {}

    void Update(float dt);
    bool IsTransitioning() const { return State != EState::Idle; }

    virtual void Render() = 0;

    // TODO: Automate this
    virtual void RenderTransitionChild(float t) { Render(); }
    virtual void RenderTransitionParent(float t) { Render(); }

    virtual void MoveObjects(Vector3 v) {}
    virtual void ScaleObjects(float scale) {}
    virtual void ResetObjectPositions() {}

    virtual Vector3 GetClosestObject(Vector3 pos) const = 0;
    virtual Vector3 GetMainObject() const = 0;

    void StartTransitionParent();
    void EndTransitionParent();

    void StartTransitionUpChild();
    void StartTransitionDownChild(Vector3 location);

    void EndTransitionUpChild();
    void EndTransitionDownChild();

    std::string Name;
    float Scale = 1.0f;
    Vector3 ParentLocationSpace;
    SandboxTarget* Parent = nullptr;
    std::unique_ptr<SandboxTarget> Child = nullptr;

    float BeginTransitionDist = 1000.0f;
    float EndTransitionDist = 400.0f;
    
protected:
    virtual void StateIdle() {}
    virtual void StateTransitioning() {}

    CShipCamera* Camera = nullptr;
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* Context = nullptr;
    DX::DeviceResources* Resources = nullptr;

private:
    enum class EState
    {
        Idle,
        TransitioningParent,
        TransitioningChild
    } State;
};