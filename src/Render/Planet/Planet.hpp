#pragma once

#include <map>
#include <list>
#include <vector>
#include <memory>
#include <typeinfo>
#include <d3d11.h>
#include <SimpleMath.h>
#include <CommonStates.h>

#include "Render/DX/RenderCommon.hpp"
#include "Render/Cameras/Camera.hpp"
#include "Render/Model/Model.hpp"

#include "Misc/Gradient.hpp"
#include "Misc/FastNoise.hpp"
#include "Components/PlanetComponent.hpp"

class TerrainHeightFunc
{
public:
    TerrainHeightFunc();
    
    void Seed(uint64_t seed);
    bool RenderUI();
    float operator()(DirectX::SimpleMath::Vector3 normal, DirectX::SimpleMath::Color& colour, int depth = 0);

    std::wstring PixelShader = L"shaders/Planet/Planet";

private:
    FastNoise Noise;

    int Octaves = 10;
    float Amplitude = 1.0f;
    float Gain = 0.5f;
    float Frequency = 1.0f;

    float AmplitudeMin = 0.5f, AmplitudeMax = 3.0f;
    float GainMin = 0.3f, GainMax = 0.8f;
    float FreqMin = 0.1f, FreqMax = 5.0f;

    Gradient::Gradient<Gradient::GradientColor> Colour;
};

class WaterHeightFunc
{
public:
    void Seed(uint64_t seed);
    bool RenderUI();
    float operator()(DirectX::SimpleMath::Vector3 normal, DirectX::SimpleMath::Color& colour, int depth = 0);

    std::wstring PixelShader = L"shaders/Planet/PlanetWater";

private:
    float Height = 0.0f;
    float HeightMin = -0.5f, HeightMax = 0.6f;
    float AlphaMin = 0.1f, AlphaMax = 0.8f;

    DirectX::SimpleMath::Color Colour;
};

class CPlanet;

class CPlanetSeeder
{
public:
    CPlanetSeeder() {}
    CPlanetSeeder(uint64_t seed);

    void SeedPlanet(CPlanet* planet) const;

    enum EType { Rocky, Habitable, GasGiant, _Max };

    uint64_t Seed;
    uint8_t Type = 0;
    float Radius = 50.0f;
    bool HasRings = false;
};

class CPlanet
{
public:
    CPlanet(ID3D11DeviceContext* context, ICamera& cam);
    ~CPlanet();

    void Update(float dt);
    void Render(float scale = 1.0f, float t = 1.0f);
    void RenderUI();
    void Move(DirectX::SimpleMath::Vector3 v);
    void Scale(float s);
    void SetScale(float s);
    void SetPosition(DirectX::SimpleMath::Vector3 p);
    
    template <class Component> bool HasComponent();
    template <class Component> Component* GetComponent();
    template <class Component> void RemoveComponent();
    void RemoveAllComponents();

    template <class Component, class... Args>
    void AddComponent(Args... args);

    float GetScale() const { return PlanetScale; }
    DirectX::SimpleMath::Vector3 GetPosition() const { return Position; }

    ID3D11Device* GetDevice() const { return Device; }
    ID3D11DeviceContext* GetContext() const { return Context; }

    std::string to_string() const;    

    uint8_t Type = 0;
    uint64_t Seed;
    ICamera& Camera;
    float Radius = 50.0f;
    float SplitDistance = 200.0f;
    std::string Name = "Planet";

    DirectX::SimpleMath::Vector3 LightSource;
    DirectX::SimpleMath::Matrix World;

    static bool Wireframe;

private:
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;

    bool Editable = false;
    float PlanetScale = 1.0f;
    DirectX::SimpleMath::Vector3 Position;

    std::list<std::unique_ptr<IPlanetComponent>> Components;
    
    void UpdateMatrix();
    void RefreshComponents();
};

template <class Component>
bool CPlanet::HasComponent()
{
    for (const auto& component : Components)
    {
        if (typeid(*component) == typeid(Component))
        {
            return true;
        }
    }

    return false;
}

template <class Component>
Component* CPlanet::GetComponent()
{
    for (const auto& component : Components)
    {
        if (typeid(*component) == typeid(Component))
        {
            return static_cast<Component*>(component.get());
        }
    }

    return nullptr;
}

template <class Component, class... Args>
void CPlanet::AddComponent(Args... args)
{
    auto component = std::make_unique<Component>(std::forward<Args>(args)...);
    component->Init();
    Components.push_back(std::move(component));

    RefreshComponents();
}

template <class Component>
void CPlanet::RemoveComponent()
{
    Components.remove_if([](const std::unique_ptr<IPlanetComponent>& component) {
        return typeid(*component) == typeid(Component);
    });

    RefreshComponents();
}