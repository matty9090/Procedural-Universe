#include "Planet.hpp"

#include <set>
#include <array>
#include <algorithm>

#include "Components/TerrainComponent.hpp"

CPlanet::CPlanet(ID3D11DeviceContext* context, ICamera* cam)
    : Camera(cam),
      Context(context)
{
    Context->GetDevice(&Device);

    World = DirectX::SimpleMath::Matrix::Identity;
    Noise.SetFractalOctaves(6);

    Components.push_back(std::make_unique<CTerrainComponent>(this));
}

CPlanet::~CPlanet()
{
    
}

void CPlanet::Update(float dt)
{
    for (auto& component : Components)
        component->Update(dt);
}

void CPlanet::Render()
{
    for (auto& component : Components)
        component->Render(Camera->GetViewMatrix() * Camera->GetProjectionMatrix());
}

void CPlanet::Move(DirectX::SimpleMath::Vector3 v)
{
    Position += v;
    UpdateMatrix();
}

void CPlanet::SetPosition(DirectX::SimpleMath::Vector3 p)
{
    Position = p;
    UpdateMatrix();
}

float CPlanet::GetHeight(DirectX::SimpleMath::Vector3 normal)
{
    normal *= 100;
    return Noise.GetSimplexFractal(normal.x, normal.y, normal.z) * 10.0f;
}

void CPlanet::UpdateMatrix()
{
    World = DirectX::SimpleMath::Matrix::CreateTranslation(Position);
}