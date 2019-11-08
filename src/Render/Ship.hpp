#pragma once

#include <Mouse.h>
#include <Keyboard.h>

#include "Model.hpp"

class CShip : public CModel
{
public:
    CShip(ID3D11Device* device, CMesh* mesh) : CModel(device, mesh) {}

    void Update(float dt);
    void Control(DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, float dt);
    float GetSpeedPercent() const { return Velocity.Length() / MaxSpeed; }

    float VelocityScale = 1.0f;

private:
    float Mass = 10.0f;
    float Accel = 0.0f;
    float Friction = 0.8f;
    float MaxSpeed = 90000.0f;
    float Thrust = 0.0f;
    float ThrustInc = 14000.0f;
    float MaxThrust = 80000.0f;
    float RotationSpeed = 0.8f;
    float ZRotationSpeed = 1.6f;

    DirectX::SimpleMath::Vector3 Velocity;
    DirectX::SimpleMath::Vector3 Momentum;
};