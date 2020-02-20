#include "Planet.hpp"

#include <set>
#include <array>
#include <algorithm>

#include "Components/TerrainComponent.hpp"
#include "Components/AtmosphereComponent.hpp"

TerrainHeightFunc::TerrainHeightFunc()
{
    Noise.SetFrequency(1.0f);
    Noise.SetFractalOctaves(6);
}

float TerrainHeightFunc::operator()(DirectX::SimpleMath::Vector3 normal)
{
    return Noise.GetSimplexFractal(normal.x, normal.y, normal.z) * Amplitude;
}

float WaterHeightFunc::operator()(DirectX::SimpleMath::Vector3 normal)
{
    return 0.0f;
}

CPlanet::CPlanet(ID3D11DeviceContext* context, ICamera& cam)
    : Camera(cam),
      Context(context)
{
    Context->GetDevice(&Device);

    World = DirectX::SimpleMath::Matrix::Identity;

    Components.push_back(std::make_unique<CAtmosphereComponent>(this, 100.0f));
    Components.push_back(std::make_unique<CTerrainComponent<TerrainHeightFunc>>(this));
    Components.push_back(std::make_unique<CTerrainComponent<WaterHeightFunc>>(this));
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
        component->Render(Camera.GetViewMatrix() * Camera.GetProjectionMatrix());
}

void CPlanet::Move(DirectX::SimpleMath::Vector3 v)
{
    Position += v;
    UpdateMatrix();
}

void CPlanet::Scale(float s)
{
    PlanetScale *= s;
    UpdateMatrix();
}

void CPlanet::SetPosition(DirectX::SimpleMath::Vector3 p)
{
    Position = p;
    UpdateMatrix();
}

void CPlanet::UpdateMatrix()
{
    World = DirectX::SimpleMath::Matrix::CreateScale(PlanetScale) *
            DirectX::SimpleMath::Matrix::CreateTranslation(Position); 
}