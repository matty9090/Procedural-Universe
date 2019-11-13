#pragma once

#include "App/DeviceResources.hpp"

#include "Core/Maths.hpp"
#include "Core/ThreadPool.hpp"

#include "Render/Model/Model.hpp"
#include "Render/Model/Skybox.hpp"
#include "Render/Cameras/ShipCamera.hpp"
#include "Render/DX/ConstantBuffer.hpp"
#include "Render/DX/RenderCommon.hpp"
#include "Render/Misc/PostProcess.hpp"
#include "Render/Misc/SkyboxGenerator.hpp"

#include <string>
#include <memory>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Services/Log.hpp"

using DirectX::SimpleMath::Vector3;

class SandboxTarget
{
public:
    SandboxTarget(ID3D11DeviceContext* context, std::string name, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv);
    virtual ~SandboxTarget() {}

    void Update(float dt);
    bool IsTransitioning() const { return State != EState::Idle; }
    Vector3 GetCentre() const { return Centre; }
    CSkyBox& GetSkyBox() { return SkyBox; }

    virtual void Render() = 0;
    virtual void RenderTransitionChild(float t) { Render(); }
    virtual void RenderTransitionParent(float t) { Render(); }

    virtual void MoveObjects(Vector3 v) {}
    virtual void ScaleObjects(float scale) {}
    virtual void ResetObjectPositions() {}

    virtual Vector3 GetClosestObject(Vector3 pos) = 0;

    void StartTransitionUpParent();
    void StartTransitionDownParent(Vector3 object);

    void EndTransitionUpParent();
    void EndTransitionDownParent(Vector3 object);

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
    virtual void OnStartTransitionUpParent() {}
    virtual void OnStartTransitionDownParent(Vector3 object) {}
    virtual void OnEndTransitionUpParent() {}
    virtual void OnEndTransitionDownParent(Vector3 object) {}
    virtual void OnStartTransitionUpChild() {}
    virtual void OnStartTransitionDownChild(Vector3 location) {}
    virtual void OnEndTransitionUpChild() {}
    virtual void OnEndTransitionDownChild() {}

    virtual void StateIdle(float dt) {}
    virtual void StateTransitioning(float dt) {}
    virtual void BakeSkybox(Vector3 object) {}

    void GenerateSkybox(Vector3 location);

    CSkyBox SkyBox;
    Vector3 Centre;
    CShipCamera* Camera = nullptr;
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* Context = nullptr;
    ID3D11RenderTargetView* RenderTarget = nullptr;
    DX::DeviceResources* Resources = nullptr;

    std::unique_ptr<CSkyboxGenerator> SkyboxGenerator;

private:
    enum class EState
    {
        Idle,
        TransitioningParent,
        TransitioningChild
    } State;
};