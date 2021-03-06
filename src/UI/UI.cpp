#include "UI.hpp"

#include "Services/Log.hpp"

#include "Core/Event.hpp"

#include <imgui.h>
#include "ImGuiDx11.h"
#include "ImGuiWin32.h"

#define UIPROPCHANGE(name, type) if(new##name != ##name) \
    { \
        ##name = new##name; \
        EventStream::Report(EEvent::##name##Changed, ##type##EventData(##name)); \
    } \

CUI::CUI(ID3D11DeviceContext* context, HWND hwnd)
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

    memset(FileBuf, '\0', 200);
}

CUI::~CUI()
{
    Reset();
}

void CUI::Render()
{
    float newSimSpeed = SimSpeed;
    int newNumParticles = NumParticles;
    float newBloomBase = BloomBase;
    float newBloomAmount = BloomAmount;
    float newBloomSat = BloomSat;
    float newBloomBaseSat = BloomBaseSat;
    float newGaussianBlur = GaussianBlur;
    float newBHTheta = BHTheta;

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
    ImGui::SliderInt("Particles", &newNumParticles, 1, 50000);
    ImGui::SliderFloat("Sim Speed", &newSimSpeed, 0.001f, 1.0f);
    
    if(ImGui::Combo("Seeder", &SelectedSeeder, "Random\0Galaxy\0StarSystem\0"))
    {
        EventStream::Report(EEvent::SeederChanged, SeederTypeEventData(SelectedSeeder));
    }

    if(ImGui::Button(DrawDebug ? "Debug Off" : "Debug On"))
    {
        DrawDebug = !DrawDebug;
        EventStream::Report(EEvent::DrawDebugChanged, BoolEventData(DrawDebug));
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

    if(SimType == ENBodySim::BarnesHut)
    {
        ImGui::SliderFloat("Theta", &newBHTheta, 0.1f, 6.0f);
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
    
    if(ImGui::Button(UseBloom ? "Bloom Off" : "Bloom On"))
    {
        UseBloom = !UseBloom;
        EventStream::Report(EEvent::UseBloomChanged, BoolEventData(UseBloom));
    }

    ImGui::SameLine();

    if(ImGui::Button(UseSplatting ? "Splatting Off" : "Splatting On"))
    {
        UseSplatting = !UseSplatting;
        EventStream::Report(EEvent::UseSplattingChanged, BoolEventData(UseSplatting));
    }

    ImGui::SliderFloat("Blur", &newGaussianBlur, 0.1f, 10.0f);
    ImGui::SliderFloat("Blm Base", &newBloomBase, 0.1f, 2.0f);
    ImGui::SliderFloat("Blm Amount", &newBloomAmount, 0.0f, 1.0f);
    ImGui::SliderFloat("Blm Saturation", &newBloomSat, 0.1f, 6.0f);
    ImGui::SliderFloat("Blm Base Sat", &newBloomBaseSat, 0.1f, 6.0f);

    ImGui::Separator();
    ImGui::Text("Precomputation");
    ImGui::InputText("File", FileBuf, 200);
    
    if(ImGui::Button("Load"))
    {
        EventStream::Report(EEvent::LoadParticleFile, StringEventData(FileBuf));
    }

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
        ImGui::Text("Force: (%.2f, %.2f, %.2f) N", SelectedParticle->Forces.x, SelectedParticle->Forces.y, SelectedParticle->Forces.z);
        ImGui::Text("Mass: %f kg", SelectedParticle->Mass);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    UIPROPCHANGE(NumParticles, Int)
    UIPROPCHANGE(SimSpeed, Float)
    UIPROPCHANGE(GaussianBlur, Float)
    UIPROPCHANGE(BloomBase, Float)
    UIPROPCHANGE(BloomAmount, Float)
    UIPROPCHANGE(BloomSat, Float)
    UIPROPCHANGE(BloomBaseSat, Float)
    UIPROPCHANGE(BHTheta, Float)

    if(runBenchmark)
    {
        EventStream::Report(EEvent::RunBenchmark, EventData {});
    }
}

void CUI::Update(float dt)
{
    ++Frames;
    FPSTimer -= dt;

    if(FPSTimer < 0.0f)
    {
        FPS = static_cast<float>(Frames) / FPSUpdate;
        Frames = 0, FPSTimer = FPSUpdate;
    }
}

void CUI::Reset()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void CUI::SetSelectedParticle(Particle* particle)
{
    SelectedParticle = particle;
}