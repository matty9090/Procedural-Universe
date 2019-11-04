#include "Ship.hpp"
#include "Services/Log.hpp"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Quaternion;

void CShip::Update(float dt)
{
    auto forward = GetForward();
    forward.Normalize();

    Accel = Thrust / Mass;
    Velocity += -forward * Accel * dt;

    if (Velocity.Length() > MaxSpeed)
    {
        Velocity.Normalize();
        Velocity *= MaxSpeed;
    }
    
    if (Velocity.Length() > 0)
    {
        auto v = Velocity;
        Velocity -= Velocity * Friction * dt;
    }

    Position += Velocity * VelocityScale * dt;
    UpdateMatrices();
}

void CShip::Control(DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, float dt)
{
    auto state = keyboard->GetState();

    if (state.IsKeyDown(DirectX::Keyboard::Space))
    {
        Thrust += ThrustInc * dt;
    }
    else if (state.IsKeyDown(DirectX::Keyboard::LeftControl))
    {
        Thrust -= ThrustInc * dt;
    }

    if (Thrust >  MaxThrust) Thrust =  MaxThrust;
    if (Thrust < -MaxThrust) Thrust = -MaxThrust;

    if (state.IsKeyDown(DirectX::Keyboard::W)) Rotation *= Quaternion::CreateFromAxisAngle(GetRight(), RotationSpeed * dt);
    if (state.IsKeyDown(DirectX::Keyboard::S)) Rotation *= Quaternion::CreateFromAxisAngle(GetRight(), -RotationSpeed * dt);
    if (state.IsKeyDown(DirectX::Keyboard::A)) Rotation *= Quaternion::CreateFromAxisAngle(GetUp(), RotationSpeed * dt);
    if (state.IsKeyDown(DirectX::Keyboard::D)) Rotation *= Quaternion::CreateFromAxisAngle(GetUp(), -RotationSpeed * dt);
    if (state.IsKeyDown(DirectX::Keyboard::Q)) Rotation *= Quaternion::CreateFromAxisAngle(GetForward(), ZRotationSpeed * dt);
    if (state.IsKeyDown(DirectX::Keyboard::E)) Rotation *= Quaternion::CreateFromAxisAngle(GetForward(), -ZRotationSpeed * dt);
}
