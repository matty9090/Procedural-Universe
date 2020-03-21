#include "Planet.hpp"

#include <set>
#include <array>
#include <chrono>
#include <random>
#include <algorithm>
#include <imgui.h>

#include "Misc/ProcUtils.hpp"
#include "Components/RingComponent.hpp"
#include "Components/TerrainComponent.hpp"
#include "Components/AtmosphereComponent.hpp"

bool CPlanet::Wireframe = false;

TerrainHeightFunc::TerrainHeightFunc()
{
    Noise.SetFrequency(Frequency);
    Noise.SetFractalGain(Gain);
    Noise.SetFractalOctaves(Octaves);
}

void TerrainHeightFunc::Seed(uint64_t seed)
{
    std::default_random_engine gen { static_cast<unsigned int>(seed) };

    std::uniform_real_distribution<float> AmpDist(AmplitudeMin, AmplitudeMax);
    std::uniform_real_distribution<float> GainDist(GainMin, GainMax);
    std::uniform_real_distribution<float> FreqDist(FreqMin, FreqMax);

    Amplitude = AmpDist(gen);
    Gain = GainDist(gen);
    Frequency = FreqDist(gen);

    auto col1 = ProcUtils::RandomColour(gen);
    auto col2 = ProcUtils::RandomColour(gen);

    Colour.AddColorStop(0.0f, Gradient::GradientColor(col1.R(), col1.G(), col1.B(), 1.0f));
    Colour.AddColorStop(1.0f, Gradient::GradientColor(col2.R(), col2.G(), col2.B(), 1.0f));
}

bool TerrainHeightFunc::RenderUI()
{
    bool dirty = false;

    dirty |= ImGui::SliderFloat("Amplitude", &Amplitude, 0.04f, 1.2f);

    if (ImGui::SliderFloat("Frequency", &Frequency, 0.1f, 4.0f))
    {
        Noise.SetFrequency(Frequency);
        dirty |= true;
    }

    if (ImGui::SliderFloat("Gain", &Gain, 0.3f, 0.8f))
    {
        Noise.SetFractalGain(Gain);
        dirty |= true;
    }

    if (ImGui::SliderInt("Octaves", &Octaves, 0, 20))
    {
        Noise.SetFractalOctaves(Octaves);
        dirty |= true;
    }

    return dirty;
}

float TerrainHeightFunc::operator()(DirectX::SimpleMath::Vector3 normal, DirectX::SimpleMath::Color& colour, int depth)
{
    auto noise = Noise.GetSimplexFractal(normal.x, normal.y, normal.z);
    auto col = Colour.GetColorAt(noise / 2.0f + 0.5f);

    colour.R(col.r);
    colour.G(col.g);
    colour.B(col.b);
    colour.A(1.0f);

    return noise * Amplitude;
}

void WaterHeightFunc::Seed(uint64_t seed)
{
    std::default_random_engine gen { static_cast<unsigned int>(seed) };
    std::uniform_real_distribution<float> HeightDist(HeightMin, HeightMax);
    std::uniform_real_distribution<float> AlphaDist(AlphaMin, AlphaMax);

    Height = HeightDist(gen);
    Colour = ProcUtils::RandomColour(gen);
    Colour.A(AlphaDist(gen));
}

bool WaterHeightFunc::RenderUI()
{
    return ImGui::SliderFloat("Water level", &Height, -0.5f, 0.6f);
}

float WaterHeightFunc::operator()(DirectX::SimpleMath::Vector3 normal, DirectX::SimpleMath::Color& colour, int depth)
{
    colour = Colour;
    return Height;
}

CPlanet::CPlanet(ID3D11DeviceContext* context, ICamera& cam)
    : Camera(cam),
      Context(context)
{
    Context->GetDevice(&Device);

    World = DirectX::SimpleMath::Matrix::Identity;
}

CPlanet::~CPlanet()
{
    
}

void CPlanet::Update(float dt)
{
    for (auto& component : Components)
        component->Update(dt);
}

void CPlanet::Render(float scale)
{
    for (auto& component : Components)
        component->Render(Camera.GetViewMatrix() * Camera.GetProjectionMatrix());
}

void CPlanet::RenderUI()
{
    ImGui::SetNextWindowSize(ImVec2(260, Camera.GetSize().y));
    ImGui::SetNextWindowPos(ImVec2(Camera.GetSize().x, 0), 0, ImVec2(1.0f, 0.0f));
    ImGui::Begin("Planet##1", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text("Seed: %i", static_cast<int>(Seed));
    ImGui::Text("Radius: %i", static_cast<int>(Radius));
    
    if (ImGui::CollapsingHeader("Planet"))
    {
        if (ImGui::Button("Randomise"))
        {
            auto t = std::chrono::system_clock::now().time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t);
            CPlanetSeeder seeder(static_cast<uint64_t>(ms.count()));
            seeder.SeedPlanet(this);
        }

        ImGui::Checkbox("Editable", &Editable);

        if (Editable)
        {
            bool chkTerr = HasComponent<CTerrainComponent<TerrainHeightFunc>>();

            if (ImGui::Checkbox("Terrain", &chkTerr))
            {
                chkTerr ? AddComponent<CTerrainComponent<TerrainHeightFunc>>(this) : RemoveComponent<CTerrainComponent<TerrainHeightFunc>>();
            }

            bool chkAtm = HasComponent<CAtmosphereComponent>();

            if (ImGui::Checkbox("Atmosphere", &chkAtm))
            {
                chkAtm ? AddComponent<CAtmosphereComponent>(this) : RemoveComponent<CAtmosphereComponent>();
            }
            
            bool chkWater = HasComponent<CTerrainComponent<WaterHeightFunc>>();

            if (ImGui::Checkbox("Water", &chkWater))
            {
                chkWater ? AddComponent<CTerrainComponent<WaterHeightFunc>>(this) : RemoveComponent<CTerrainComponent<WaterHeightFunc>>();
            }

            bool chkRings = HasComponent<CRingComponent>();

            if (ImGui::Checkbox("Rings", &chkRings))
            {
                chkRings ? AddComponent<CRingComponent>(this) : RemoveComponent<CRingComponent>();
            }
        }
    }

    ImGui::BeginChild("Components");

    if (Editable)
    {
        for (auto& component : Components)
            component->RenderUI();
    }

    ImGui::EndChild();
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

void CPlanet::RemoveAllComponents()
{
    Components.clear();
}

std::string CPlanet::to_string() const
{
    std::string type = (Type == CPlanetSeeder::Rocky) ? "Rocky" : (Type == CPlanetSeeder::Habitable ? "Habitable" : "Gas Giant");

    std::ostringstream ss;
    ss << "Planet " << Name << " (" << type << ")\n";
    
    for (const auto& c : Components)
    {
        ss << "\tComponent [" << c->GetName() << "]\n";
    }

    return ss.str();
}

void CPlanet::UpdateMatrix()
{
    World = DirectX::SimpleMath::Matrix::CreateScale(PlanetScale) *
            DirectX::SimpleMath::Matrix::CreateTranslation(Position); 
}

void CPlanet::RefreshComponents()
{
    for (auto& c : Components)
        c->Init();
}

CPlanetSeeder::CPlanetSeeder(uint64_t seed) : Seed(seed)
{
    auto gen = std::default_random_engine { static_cast<unsigned int>(seed) };
    std::uniform_int_distribution<> distType(0, EType::_Max - 1);

    Type = distType(gen);

    // Habitable
    if (Type == EType::Habitable)
    {
        std::uniform_real_distribution<float> distRadius(40.0f, 70.0f);
        Radius = distRadius(gen);
    }
    // Gas Giant
    else if (Type == EType::GasGiant)
    {
        std::uniform_real_distribution<float> distRadius(220.0f, 400.0f);
        Radius = distRadius(gen);
    }
    // Rocky
    else if (Type == EType::Rocky)
    {
        std::uniform_real_distribution<float> distRadius(40.0f, 70.0f);
        Radius = distRadius(gen);
    }
}

void CPlanetSeeder::SeedPlanet(CPlanet* planet) const
{
    planet->RemoveAllComponents();

    planet->Seed = Seed;
    planet->Type = Type;
    planet->Radius = Radius;

    switch (Type)
    {
    case Habitable:
        planet->AddComponent<CAtmosphereComponent>(planet, Seed);
        planet->AddComponent<CTerrainComponent<WaterHeightFunc>>(planet, Seed);
        planet->AddComponent<CTerrainComponent<TerrainHeightFunc>>(planet, Seed);
        break;

    case GasGiant:
        planet->AddComponent<CAtmosphereComponent>(planet, Seed);
        planet->AddComponent<CTerrainComponent<WaterHeightFunc>>(planet, Seed);
        if (HasRings) planet->AddComponent<CRingComponent>(planet, Seed);
        break;

    case Rocky:
        planet->AddComponent<CTerrainComponent<TerrainHeightFunc>>(planet, Seed);
        break;
    }

    LOGV(planet->to_string())
}
