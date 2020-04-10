#include "BlankComponent.hpp"
#include "Render/Planet/Planet.hpp"
#include "Services/Log.hpp"

#include <random>
#include <imgui.h>

CBlankComponent::CBlankComponent(CPlanet* planet, uint64_t seed) : Planet(planet)
{

}

void CBlankComponent::LoadCache(ID3D11Device* device)
{

}

void CBlankComponent::Init()
{

}

void CBlankComponent::Update(float dt)
{

}

void CBlankComponent::Render(DirectX::SimpleMath::Matrix viewProj, float t)
{
    
}

void CBlankComponent::RenderUI()
{
    
}