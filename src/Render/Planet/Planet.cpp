#include "Planet.hpp"

#include <set>
#include <array>
#include <algorithm>
#include <imgui.h>

#include "Components/TerrainComponent.hpp"
#include "Components/AtmosphereComponent.hpp"

bool CPlanet::Wireframe = false;

TerrainHeightFunc::TerrainHeightFunc()
{
    Noise.SetFrequency(1.0f);
    Noise.SetFractalOctaves(10);
}

float TerrainHeightFunc::operator()(DirectX::SimpleMath::Vector3 normal, int depth)
{
    return Noise.GetSimplexFractal(normal.x, normal.y, normal.z) * Amplitude;
}

float WaterHeightFunc::operator()(DirectX::SimpleMath::Vector3 normal, int depth)
{
    return 0.0f;
}

CPlanet::CPlanet(ID3D11DeviceContext* context, ICamera& cam)
    : Camera(cam),
      Context(context)
{
    Context->GetDevice(&Device);

    World = DirectX::SimpleMath::Matrix::Identity;
    
    Components.push_back(std::make_unique<CAtmosphereComponent>(this));
    Components.push_back(std::make_unique<CTerrainComponent<TerrainHeightFunc>>(this));
    Components.push_back(std::make_unique<CTerrainComponent<WaterHeightFunc>>(this));
}

CPlanet::~CPlanet()
{
    
}

void CPlanet::Seed(uint64_t seed)
{

}

void CPlanet::Update(float dt)
{
    for (auto& component : Components)
        component->Update(dt);
}

void CPlanet::Render(float scale)
{
    /*Matrix view = Camera.GetViewMatrix();
    Matrix viewProj = view * Camera.GetProjectionMatrix();

    view = view.Invert();
    view *= Matrix::CreateScale(scale);
    view = view.Invert();*/

    for (auto& component : Components)
        component->Render(Camera.GetViewMatrix() * Camera.GetProjectionMatrix());
}

void CPlanet::RenderUI()
{
    ImGui::SetNextWindowSize(ImVec2(260, Camera.GetSize().y));
    ImGui::SetNextWindowPos(ImVec2(Camera.GetSize().x, 0), 0, ImVec2(1.0f, 0.0f));
    ImGui::Begin("Planet", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    if (ImGui::CollapsingHeader("Planet"))
    {
        static bool chkAtm = HasComponent<CAtmosphereComponent>();
        static bool chkWater = HasComponent<CTerrainComponent<WaterHeightFunc>>();

        /*if (ImGui::Checkbox("Atmosphere", &chkAtm))
        {
            if (chkAtm)
                Components.push_back(std::make_unique<CAtmosphereComponent>(this));
            else
                RemoveComponent<CAtmosphereComponent>();
        }*/

        if (ImGui::Checkbox("Water", &chkWater))
        {
            if (chkWater)
                Components.push_back(std::make_unique<CTerrainComponent<WaterHeightFunc>>(this));
            else
                RemoveComponent<CTerrainComponent<WaterHeightFunc>>();
        }

        if (ImGui::Button("Move forwards"))
        {
            Move(Vector3(10.0f, 0.0f, 0.0f));
        }

        if (ImGui::Button("Move backwards"))
        {
            Move(Vector3(-10.0f, 0.0f, 0.0f));
        }
    }

    for (auto& component : Components)
        component->RenderUI();

    ImGui::End();
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

void CPlanet::SetScale(float s)
{
    PlanetScale = s;
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