#include "UI.hpp"

#include "Services/Log.hpp"

#include "Core/Event.hpp"

#include <imgui.h>
#include "ImGuiDx11.h"
#include "ImGuiWin32.h"

UI::UI(ID3D11DeviceContext* context, HWND hwnd)
{
    ID3D11Device* device = nullptr;
    context->GetDevice(&device);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    EventStream::Register(EEvent::BenchmarkResult, [this](const EventData& data) {
        auto& bdata = static_cast<const BenchmarkEventData&>(data);
        BenchmarkData[bdata.SimType] = bdata;
    });

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
}

UI::~UI()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void UI::Render()
{
    float newSimSpeed = SimSpeed;
    int newParticles = Particles;
    float newBloomBase = BloomBase;
    float newBloomAmount = BloomAmount;
    float newGaussianBlur = GaussianBlur;
    bool runBenchmark = false;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(260, io.DisplaySize.y));

    ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    ImGui::Text("FPS: %i", (int)FPS);
    ImGui::Separator();

    ImGui::Text("Settings");
    ImGui::SliderInt("Particles", &newParticles, 1, 10000);
    ImGui::SliderFloat("Sim Speed", &newSimSpeed, 0.01f, 10.0f);
    
    if(ImGui::Combo("Seeder", &SelectedSeeder, "Random\0Galaxy\0StarSystem\0"))
    {
        EventStream::Report(EEvent::SeederChanged, SeederTypeEventData(static_cast<EParticleSeeder>(SelectedSeeder)));
    }

    ImGui::Separator();
    ImGui::Text("Simulators");

    for(int sim = 0; sim < static_cast<int>(ENBodySim::NumSims); ++sim)
    {
        SimType = static_cast<ENBodySim>(sim);

        if(sim % 2 != 0)
            ImGui::SameLine();

        if(ImGui::Button(NBodySimGetName(SimType).c_str()))
        {
            EventStream::Report(EEvent::SimTypeChanged, SimTypeEventData(SimType));
        }
    }

    ImGui::Separator();
    ImGui::Text("Frame options");

    if(ImGui::Button(Paused ? "Play simulation" : "Pause simulation"))
    {
        Paused = !Paused;
        EventStream::Report(EEvent::IsPausedChanged, BoolEventData(Paused));
    }

    ImGui::SameLine();

    if(ImGui::Button("Force frame"))
    {
        EventStream::Report(EEvent::ForceFrame, FloatEventData(1.0f / 60.0f));
    }

    ImGui::Separator();
    ImGui::Text("Post processing");
    ImGui::SliderFloat("Blur", &newGaussianBlur, 0.1f, 12.0f);
    ImGui::SliderFloat("Bloom Base", &newBloomBase, 0.1f, 3.0f);
    ImGui::SliderFloat("Bloom Amount", &newBloomAmount, 0.1f, 6.0f);

    ImGui::Separator();
    ImGui::Text("Benchmarking");

    if(ImGui::Button("Run benchmark"))
    {
        runBenchmark = true;
    }

    for(const auto& data : BenchmarkData)
    {
        ImGui::Text("%s: %i ms", NBodySimGetName(data.first).c_str(), data.second.Time);
    }

    if(SelectedParticle)
    {
        ImGui::Separator();
        ImGui::Text("Particle infomation");
        ImGui::Text("Position: (%.0f, %.0f, %.0f)", SelectedParticle->Position.x, SelectedParticle->Position.y, SelectedParticle->Position.z);
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", SelectedParticle->Velocity.x, SelectedParticle->Velocity.y, SelectedParticle->Velocity.z);
        ImGui::Text("Accel: (%.2f, %.2f, %.2f) N", SelectedParticle->Accel.x, SelectedParticle->Accel.y, SelectedParticle->Accel.z);
        ImGui::Text("Mass: %f kg", SelectedParticle->Mass);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if(newParticles != Particles)
    {
        Particles = newParticles;
        EventStream::Report(EEvent::NumParticlesChanged, IntEventData(Particles));
    }

    if(newSimSpeed != SimSpeed)
    {
        SimSpeed = newSimSpeed;
        EventStream::Report(EEvent::SimSpeedChanged, FloatEventData(SimSpeed));
    }

    if(newGaussianBlur != GaussianBlur)
    {
        GaussianBlur = newGaussianBlur;
        EventStream::Report(EEvent::GaussianBlurChanged, FloatEventData(GaussianBlur));
    }

    if(newBloomBase != BloomBase)
    {
        BloomBase = newBloomBase;
        EventStream::Report(EEvent::BloomBaseChanged, FloatEventData(BloomBase));
    }

    if(newBloomAmount != BloomAmount)
    {
        BloomAmount = newBloomAmount;
        EventStream::Report(EEvent::BloomAmountChanged, FloatEventData(BloomAmount));
    }

    if(runBenchmark)
    {
        EventStream::Report(EEvent::RunBenchmark, EventData {});
    }
}

void UI::Update(float dt)
{
    ++Frames;
    FPSTimer -= dt;

    if(FPSTimer < 0.0f)
    {
        FPS = static_cast<float>(Frames) / FPSUpdate;
        Frames = 0, FPSTimer = FPSUpdate;
    }
}

void UI::Reset()
{

}

void UI::SetSelectedParticle(Particle* particle)
{
    SelectedParticle = particle;
}