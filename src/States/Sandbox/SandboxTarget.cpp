#include "SandboxTarget.hpp"
#include "Services/Log.hpp"

SandboxTarget::SandboxTarget(ID3D11DeviceContext* context, std::string name, DX::DeviceResources* resources, CShipCamera* camera)
    : Context(context),
      Name(name),
      State(EState::Idle),
      Resources(resources),
      Camera(camera),
      SkyBox(context),
      SkyboxWorker(std::bind(&SandboxTarget::BakeSkybox, this, std::placeholders::_1))
{
    context->GetDevice(&Device);

    auto size = resources->GetOutputSize();
    SkyboxGenerator = std::make_unique<CSkyboxGenerator>(Device, Context, size.right - size.left, size.bottom - size.top);
}

void SandboxTarget::Update(float dt)
{
    switch (State)
    {
        case EState::Idle:
            StateIdle();
            break;

        case EState::TransitioningParent:
        case EState::TransitioningChild:
            StateTransitioning();
            break;
    }
}

void SandboxTarget::StartTransitionUpParent()
{
    State = EState::TransitioningParent;
}

void SandboxTarget::StartTransitionDownParent(Vector3 object)
{
    State = EState::TransitioningParent;
    //SkyboxWorker.Dispatch(1, object);
}

void SandboxTarget::EndTransitionUpParent()
{
    State = EState::Idle;
}

void SandboxTarget::EndTransitionDownParent(Vector3 object)
{
    State = EState::Idle;
    //SkyboxWorker.Join();
    BakeSkybox(object);
    SkyBox.SetTextureReceiveOwnership(SkyboxGenerator->GetTextureTakeOwnership());
    SkyBox.SetPosition(object);
}

void SandboxTarget::StartTransitionUpChild()
{
    State = EState::TransitioningChild;
    ResetObjectPositions();
    ScaleObjects(1.0f / Scale);
}

void SandboxTarget::StartTransitionDownChild(Vector3 location)
{
    State = EState::TransitioningChild;
    ParentLocationSpace = location;
    ScaleObjects(1.0f / Scale);
}

void SandboxTarget::EndTransitionUpChild()
{
    State = EState::Idle;
    ScaleObjects(Scale);
}

void SandboxTarget::EndTransitionDownChild()
{
    State = EState::Idle;
    ScaleObjects(Scale);
}