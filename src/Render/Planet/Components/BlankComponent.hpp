#pragma once

#include <map>
#include <array>
#include <vector>
#include <memory>
#include <CommonStates.h>

#include "Render/DX/RenderCommon.hpp"
#include "Render/DX/ConstantBuffer.hpp"
#include "PlanetComponent.hpp"

class CPlanet;

class CBlankComponent : public IPlanetComponent
{
public:
    CBlankComponent(CPlanet* planet, uint64_t seed = 0);

    void Init() final;
    void Update(float dt) final;
    void Render(DirectX::SimpleMath::Matrix viewProj, float t) final;
    void RenderUI() final;

    std::string GetName() const override { return "Rings"; }

private:
    CPlanet* Planet;
};