#include "Ship.hpp"
#include "Services/Log.hpp"

using DirectX::SimpleMath::Vector3;

void CShip::Update(float dt)
{
    Velocity += -GetForward() * Accel * dt;

    if (Velocity.Length() > MaxSpeed)
    {
        Velocity.Normalize();
        Velocity *= MaxSpeed;
    }
    
    if (Velocity.Length() > 0)
    {
        auto v = Velocity;
        v.Normalize();
        Velocity -= v * Friction * dt;

        FLog::Get().Log(Velocity);
    }

    if (Velocity.Dot(GetForward()) > 0)
    {
        Velocity = Vector3::Zero;
    }

    Position += Velocity * dt;
}

void CShip::Control(DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, float dt)
{
    auto state = keyboard->GetState();

    Accel = 0.0f;

    if (state.IsKeyDown(DirectX::Keyboard::Space))
    {
        Accel = Thrust / Mass;
    }
    else if (state.IsKeyDown(DirectX::Keyboard::LeftControl))
    {
        Accel = -Thrust / Mass;
    }

    if (state.IsKeyDown(DirectX::Keyboard::W))
    {
        Rotate(Vector3(-RotationSpeed * dt, 0.0f, 0.0f));
    }
    if (state.IsKeyDown(DirectX::Keyboard::S))
    {
        Rotate(Vector3(RotationSpeed * dt, 0.0f, 0.0f));
    }

    if (state.IsKeyDown(DirectX::Keyboard::A))
    {
        Rotate(Vector3(0.0f, RotationSpeed * dt, 0.0f));
    }
    if (state.IsKeyDown(DirectX::Keyboard::D))
    {
        Rotate(Vector3(0.0f, -RotationSpeed * dt, 0.0f));
    }
}
