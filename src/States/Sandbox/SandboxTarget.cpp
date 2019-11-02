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
        case EState::Idle: StateIdle(); break;
        case EState::Transitioning: StateTransitioning(); break;
    }
}

void SandboxTarget::RenderTransition(float t)
{

}

void SandboxTarget::BeginTransition(Vector3 position)
{
    Location = position;
    State = EState::Transitioning;
}

void SandboxTarget::EndTransition()
{
    State = EState::Idle;
}