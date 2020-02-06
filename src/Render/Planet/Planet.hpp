#pragma once

#include <map>
#include <list>
#include <vector>
#include <memory>
#include <d3d11.h>
#include <SimpleMath.h>
#include <CommonStates.h>

#include "Render/DX/RenderCommon.hpp"
#include "Render/Cameras/Camera.hpp"

#include "Misc/FastNoise.hpp"
#include "Components/PlanetComponent.hpp"

class CTerrainComponent;

class CPlanet
{
public:
    CPlanet(ID3D11DeviceContext* context, ICamera* cam);
    ~CPlanet();

    void Update(float dt);
    void Render();
    void Move(DirectX::SimpleMath::Vector3 v);
    void Scale(float s);
    void SetPosition(DirectX::SimpleMath::Vector3 p);
    
    float GetScale() const { return PlanetScale; }
    float GetHeight(DirectX::SimpleMath::Vector3 normal);

    DirectX::SimpleMath::Vector3 GetPosition() const { return Position; }

    ID3D11Device* GetDevice() const { return Device; }
    ID3D11DeviceContext* GetContext() const { return Context; }
    
    ICamera* Camera;
    float Radius = 250.0f;
    float SplitDistance = 1500.0f;
    DirectX::SimpleMath::Matrix World;

private:
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;

    FastNoise Noise;
    float PlanetScale = 1.0f;
    DirectX::SimpleMath::Vector3 Position;

    std::list<std::unique_ptr<IPlanetComponent>> Components;
    
    void UpdateMatrix();
};