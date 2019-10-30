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

private:
    float Mass = 10.0f;
    float Accel = 0.0f;
    float Friction = 40.0f;
    float MaxSpeed = 60.0f;
    float Thrust = 0.0f;
    float ThrustInc = 300.0f;
    float MaxThrust = 200.0f;
    float RotationSpeed = 0.8f;

    DirectX::SimpleMath::Vector3 Velocity;
    DirectX::SimpleMath::Vector3 Momentum;
    
    float Yaw = 0.0f, Pitch = 0.0f, Roll = 0.0f;
};