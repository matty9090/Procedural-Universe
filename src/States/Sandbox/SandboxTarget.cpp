#include "SandboxTarget.hpp"
#include "Services/Log.hpp"

SandboxTarget::SandboxTarget(ID3D11DeviceContext* context, std::string name, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv)
    : Context(context),
      Name(name),
      State(EState::Idle),
      Resources(resources),
      Camera(camera),
      SkyBox(context),
      RenderTarget(rtv)
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
    OnStartTransitionUpParent();
}

void SandboxTarget::StartTransitionDownParent(Vector3 object)
{
    State = EState::TransitioningParent;
    OnStartTransitionDownParent(object);
}

void SandboxTarget::EndTransitionUpParent()
{
    State = EState::Idle;
    OnEndTransitionUpParent();
}

void SandboxTarget::EndTransitionDownParent(Vector3 object)
{
    State = EState::Idle;
    OnEndTransitionDownParent(object);
}

void SandboxTarget::StartTransitionUpChild()
{
    State = EState::TransitioningChild;
    ResetObjectPositions();
    ScaleObjects(1.0f / Scale);
    OnStartTransitionUpChild();
}

void SandboxTarget::StartTransitionDownChild(Vector3 location)
{
    State = EState::TransitioningChild;
    ParentLocationSpace = location;
    ScaleObjects(1.0f / Scale);
    OnStartTransitionDownChild(location);
}

void SandboxTarget::EndTransitionUpChild()
{
    State = EState::Idle;
    ScaleObjects(Scale);
    OnEndTransitionUpChild();
}

void SandboxTarget::EndTransitionDownChild()
{
    State = EState::Idle;
    ScaleObjects(Scale);
    OnEndTransitionDownChild();
}

void SandboxTarget::GenerateSkybox(Vector3 location)
{
    SkyboxGenerator->SetPosition(location);
    BakeSkybox(location);
    SkyBox.SetTextureReceiveOwnership(SkyboxGenerator->GetTextureTakeOwnership());
}
