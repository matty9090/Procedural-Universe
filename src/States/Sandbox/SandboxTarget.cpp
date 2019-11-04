#include "SandboxTarget.hpp"
#include "Services/Log.hpp"

SandboxTarget::SandboxTarget(ID3D11DeviceContext* context, std::string name, DX::DeviceResources* resources, CShipCamera* camera)
    : Context(context),
      Name(name),
      State(EState::Idle),
      Resources(resources),
      Camera(camera)
{
    context->GetDevice(&Device);
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

void SandboxTarget::StartTransitionParent()
{
    State = EState::TransitioningParent;
    //ResetObjectPositions();
    //ScaleObjects(Scale);
}

void SandboxTarget::EndTransitionParent()
{
    State = EState::Idle;
    //ScaleObjects(1.0f / Scale);
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