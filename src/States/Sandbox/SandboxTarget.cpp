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

void SandboxTarget::BeginTransition()
{
    LOGM("Starting transition from " + Name + " to " + Child->Name)
    State = EState::Transitioning;
    OnBeginTransition();
}

void SandboxTarget::EndTransition()
{
    LOGM("Transitioned from " + Name + " to " + Child->Name)
    State = EState::Idle;
    OnEndTransition();
}