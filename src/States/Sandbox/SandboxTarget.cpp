#include "SandboxTarget.hpp"
#include "Services/Log.hpp"
#include "Core/Timer.hpp"

SandboxTarget::SandboxTarget(ID3D11DeviceContext* context, std::string name, std::string objName, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : Context(context),
      Name(name),
      ObjName(objName),
      State(EState::Idle),
      Resources(resources),
      Camera(camera),
      SkyBox(context),
      RenderTarget(rtv),
      Pool(std::bind(&SandboxTarget::Worker, this, std::placeholders::_1))
{
    Context->GetDevice(&Device);
    
    auto size = resources->GetOutputSize();
    SkyboxGenerator = std::make_unique<CSkyboxGenerator>(Device, Context, size.right - size.left, size.bottom - size.top);
}

void SandboxTarget::Update(float dt)
{
    switch (State)
    {
        case EState::Idle:
            StateIdle(dt);
            break;

        case EState::TransitioningParent:
        case EState::TransitioningChild:
            StateTransitioning(dt);
            break;
    }
}

void SandboxTarget::StartTransitionUpParent()
{
    CTimer timer("start transition up (parent)");
    State = EState::TransitioningParent;
    OnStartTransitionUpParent();
}

void SandboxTarget::StartTransitionDownParent(Vector3 object)
{
    CTimer timer("start transition down (parent)");
    State = EState::TransitioningParent;
    OnStartTransitionDownParent(object);
}

void SandboxTarget::EndTransitionUpParent()
{
    CTimer timer("end transition up (parent)");
    State = EState::Idle;
    OnEndTransitionUpParent();
}

void SandboxTarget::EndTransitionDownParent(Vector3 object)
{
    CTimer timer("end transition down (parent)");
    State = EState::Idle;
    OnEndTransitionDownParent(object);
}

void SandboxTarget::StartTransitionUpChild()
{
    CTimer timer("start transition up (child)");
    State = EState::TransitioningChild;

    if (RenderParentInChildSpace)
    {
        Parent->MoveObjects(Vector3(ParentInChildSpace / Scale) - ParentOffset);
        Parent->ScaleObjects(1.0f / Scale);
        Parent->GetClosestObject(Camera->GetPosition());
    }

    ResetObjectPositions();
    ScaleObjects(1.0f / Scale);

    OnStartTransitionUpChild();
}

void SandboxTarget::StartTransitionDownChild(Vector3 location, uint64_t seed = 0)
{
    CTimer timer("start transition down (child)");
    State = EState::TransitioningChild;
    ParentLocationSpace = location;
    Seed(seed);
    SeedValue = seed;
    ScaleObjects(1.0f / Scale);
    OnStartTransitionDownChild(location);
}

void SandboxTarget::EndTransitionUpChild()
{
    CTimer timer("end transition up (child)");
    State = EState::Idle;
    ScaleObjects(Scale);
    OnEndTransitionUpChild();
    GetClosestObject(Camera->GetPosition());
}

void SandboxTarget::EndTransitionDownChild()
{
    CTimer timer("end transition down (child)");
    State = EState::Idle;
    ScaleObjects(Scale);

    if (RenderParentInChildSpace)
    {
        ParentOffset = Vector3::Zero;
        ParentInChildSpace = Parent->GetClosestObject(Camera->GetPosition());
        Parent->ScaleObjects(Scale);
        Parent->MoveObjects(-ParentInChildSpace / Scale);
    }

    OnEndTransitionDownChild();
}

void SandboxTarget::GenerateSkybox(Vector3 location)
{
    CTimer timer("skybox");
    SkyboxGenerator->SetPosition(location);
    BakeSkybox(location);
    SkyBox.SetTextureReceiveOwnership(SkyboxGenerator->GetTextureTakeOwnership());
}

void SandboxTarget::RenderParentSkybox()
{
    if (Parent)
    {
        Context->OMSetRenderTargets(1, &RenderTarget, Resources->GetDepthStencilView());
        Parent->GetSkyBox().Draw(Camera->GetViewMatrix() * Camera->GetProjectionMatrix());
    }
}

void SandboxTarget::DispatchTask(EWorkerTask task, std::function<void()> func)
{
    CTimer timer("dispatch");

    auto id = static_cast<uint32_t>(task);

    if (Pool.IsWorking(id))
    {
        CTimer timer("finishing task " + std::to_string(static_cast<int>(task)));
        Pool.Join(static_cast<uint32_t>(task));
    }

    Pool.Dispatch(id, func);
}

void SandboxTarget::FinishTask(EWorkerTask task)
{
    CTimer timer("finishing task " + std::to_string(static_cast<int>(task)));
    Pool.Join(static_cast<uint32_t>(task));
}

void SandboxTarget::Worker(std::function<void()> func)
{
    func();
}
