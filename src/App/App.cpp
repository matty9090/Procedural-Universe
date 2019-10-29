//
// App.cpp
//

#include "App/AppCore.hpp"
#include "App/App.hpp"

#include "Core/Event.hpp"
#include "Core/Except.hpp"

#include "Render/Shader.hpp"
#include "Render/RenderCommon.hpp"

#include "Services/Log.hpp"

#include <SimpleMath.h>
#include <fstream>
#include <direct.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

App::App() noexcept(false)
{
    DeviceResources = std::make_unique<DX::DeviceResources>();
    DeviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void App::Initialize(HWND window, int width, int height)
{
    FLog::Get().Log("Initializing...");

    Seeder = CreateParticleSeeder(Particles, EParticleSeeder::Random);

    Mouse = std::make_unique<DirectX::Mouse>();
    Mouse->SetWindow(window);

    DeviceResources->SetWindow(window, width, height);

    DeviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    DeviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    Timer.SetFixedTimeStep(true);
    Timer.SetTargetElapsedSeconds(1.0 / 60.0);

    RegisterEvents();

    FLog::Get().Log("Initialized");
}

void App::RegisterEvents()
{
    EventStream::Register(EEvent::SimSpeedChanged, [this](const EventData& data) {
        SimSpeed = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::NumParticlesChanged, [this](const EventData& data) {
        NumParticles = static_cast<const IntEventData&>(data).Value;
        InitParticles();
    });

    EventStream::Register(EEvent::SimTypeChanged, [this](const EventData& data) {
        Sim.reset();
        Sim = CreateNBodySim(DeviceResources->GetD3DDeviceContext(), static_cast<const SimTypeEventData&>(data).Value);
        Sim->Init(Particles);
    });

    EventStream::Register(EEvent::IsPausedChanged, [this](const EventData& data) {
        bIsPaused = static_cast<const BoolEventData&>(data).Value;
    });

    EventStream::Register(EEvent::SeederChanged, [this](const EventData& data) {
        Seeder = CreateParticleSeeder(Particles, static_cast<const SeederTypeEventData&>(data).Value);
        InitParticles();
    });

    EventStream::Register(EEvent::ForceFrame, [this](const EventData& data) {
        float dt = static_cast<const FloatEventData&>(data).Value;
        Sim->Update(dt * SimSpeed);
    });

    EventStream::Register(EEvent::RunBenchmark, [this](const EventData& data) {
        RunBenchmark();
    });

    EventStream::Register(EEvent::DrawDebugChanged, [this](const EventData& data) {
        bDrawDebug = static_cast<const BoolEventData&>(data).Value;
    });

    EventStream::Register(EEvent::TrackParticle, [this](const EventData& data) {
        auto p = static_cast<const ParticleEventData&>(data).Value;
        Camera->Track(p);
        FLog::Get().Log("Tracking particle");
    });

    EventStream::Register(EEvent::LoadParticleFile, [this](const EventData& data) {
        if (InitParticlesFromFile(static_cast<const StringEventData&>(data).Value, Particles))
        {
            NumParticles = static_cast<unsigned int>(Particles.size());

            Sim->Init(Particles);
            ParticleBuffer.Reset();

            D3D11_BUFFER_DESC buffer;
            buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            buffer.Usage = D3D11_USAGE_DEFAULT;
            buffer.ByteWidth = NumParticles * sizeof(Particle);
            buffer.CPUAccessFlags = 0;
            buffer.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA init;
            init.pSysMem = &Particles[0];

            DX::ThrowIfFailed(DeviceResources->GetD3DDevice()->CreateBuffer(&buffer, &init, ParticleBuffer.ReleaseAndGetAddressOf()));
        }
    });

    EventStream::Register(EEvent::UseSplattingChanged, [this](const EventData& data) {
        bUseSplatting = static_cast<const BoolEventData&>(data).Value;
    });
}

void App::InitParticles()
{
    UI->SetSelectedParticle(nullptr);
    Particles.resize(NumParticles);
    Seeder->Seed();
    Sim->Init(Particles);
    ParticleBuffer.Reset();

    CreateParticleBuffer(DeviceResources->GetD3DDevice(), ParticleBuffer.ReleaseAndGetAddressOf(), Particles);
}

bool App::InitParticlesFromFile(std::string fname, std::vector<Particle>& particles)
{
    std::ifstream infile("data/" + fname, std::ios::binary);

    if(!infile.is_open())
    {
        FLog::Get().Log("Could not read particle file " + fname, FLog::Error);
        return false;
    }

    particles.clear();

    while(true)
    {
        Particle p;
        infile.read(reinterpret_cast<char*>(&p), sizeof(Particle));

        if(!infile)
            break;
        
        particles.push_back(p);
    }

    infile.close();

    FLog::Get().Log("Read " + std::to_string(particles.size()) + " particles from file");
    
    long double TotalMass = 0.0;
    Vec3d CentreOfMass;

    for(const auto& particle : particles)
    {
        Vec3d pos(particle.Position.x, particle.Position.y, particle.Position.z);

        TotalMass += particle.Mass;
        CentreOfMass += pos * particle.Mass;
    }

    CentreOfMass /= TotalMass;

    DirectX::SimpleMath::Vector3 Centre(static_cast<float>(CentreOfMass.x),
                                        static_cast<float>(CentreOfMass.y),
                                        static_cast<float>(CentreOfMass.z));

    for(auto& particle : particles)
        particle.Position -= Centre;

    return true;
}

void App::RunSimulation(float dt, int time, int numparticles, std::string fileName)
{
    std::vector<Particle> particles(numparticles);

    if (fileName.length() > 0)
        InitParticlesFromFile(fileName, particles);

    auto sim = CreateNBodySim(nullptr, ENBodySim::BarnesHut);
    auto seeder = CreateParticleSeeder(particles, EParticleSeeder::StarSystem);
    
    seeder->Seed();
    sim->Init(particles);

    LARGE_INTEGER startTime, endTime, timer, tstart;
    QueryPerformanceCounter(&startTime);
    QueryPerformanceCounter(&tstart);

    long iterations = 0;

    while(true)
    {
        ++iterations;

        QueryPerformanceCounter(&endTime);
        timer = endTime;

        if(static_cast<int>((endTime.QuadPart - startTime.QuadPart) / 10000000) >= time)
            break;

        if(static_cast<int>(((timer.QuadPart - tstart.QuadPart) / 10000000) >= 1))
        {
            QueryPerformanceCounter(&tstart);
            FLog::Get().Log("Running... (" + std::to_string(iterations) + " iterations)");
        }
        
        sim->Update(dt);
    }

    _mkdir("data");
    auto filename = "data/" + std::to_string(endTime.QuadPart) + ".nbody";
    std::ofstream file(filename, std::ios::binary);
    
    if(file.is_open())
    {
        for(const auto& p : particles)
        {
            file.write(reinterpret_cast<const char*>(&p), sizeof(p));
        }

        file.close();
    }
}

#pragma region Frame Update
// Executes the basic game loop.
void App::Tick()
{
    Timer.Tick([&]()
    {
        Update(static_cast<float>(Timer.GetElapsedSeconds()));
    });

    Render();
}

// Updates the world.
void App::Update(float dt)
{
    auto mouse_state = Mouse->GetState();

    CheckParticleSelected(mouse_state);

    Camera->Events(Mouse.get(), mouse_state, dt);
    Camera->Update(dt);
    UI->Update(dt);

    if(!bIsPaused)
        Sim->Update(dt * SimSpeed);

    auto context = DeviceResources->GetD3DDeviceContext();
    context->UpdateSubresource(ParticleBuffer.Get(), 0, NULL, &Particles[0], 0, 0);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void App::Render()
{
    if (Timer.GetFrameCount() == 0)
        return;

    DeviceResources->PIXBeginEvent(L"Render");

    Clear();

    auto context = DeviceResources->GetD3DDeviceContext();
    auto sceneTarget = DeviceResources->GetSceneRenderTargetView();
    auto renderTarget = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView();

    context->OMSetRenderTargets(1, &sceneTarget, dsv);
    
    // HACK
    RenderParticles();

    if(bUseSplatting)
        Splatting->Render(NumParticles, DeviceResources->GetSceneShaderResourceView());
    
    RenderParticles();

    context->GSSetShader(nullptr, 0, 0);
    
    if(bDrawDebug)
        Sim->RenderDebug(Camera->GetViewMatrix(), Camera->GetProjectionMatrix());

    PostProcess->Render(renderTarget, dsv, DeviceResources->GetSceneShaderResourceView());

    UI->Render();

    DeviceResources->PIXEndEvent();
    DeviceResources->Present();
}

void App::RenderParticles()
{
    auto context = DeviceResources->GetD3DDeviceContext();

    context->VSSetShader(VertexShader.Get(), 0, 0);
    context->GSSetShader(GeometryShader.Get(), 0, 0);
    context->PSSetShader(PixelShader.Get(), 0, 0);

    unsigned int offset = 0;
    unsigned int stride = sizeof(Particle);

    context->IASetInputLayout(InputLayout.Get());
    context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    
    DirectX::SimpleMath::Matrix view = Camera->GetViewMatrix();
    DirectX::SimpleMath::Matrix proj = Camera->GetProjectionMatrix();

    GSBuffer->SetData(context, Buffers::GS { view * proj, view.Invert() });
    context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
    context->RSSetState(DeviceResources->GetRasterizerState());
    context->OMSetBlendState(CommonStates->Additive(), Colors::Black, 0xFFFFFFFF);
    context->Draw(NumParticles, 0);
}

// Helper method to clear the back buffers.
void App::Clear()
{
    DeviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = DeviceResources->GetD3DDeviceContext();

    auto depthStencil = DeviceResources->GetDepthStencilView();
    auto renderTarget = DeviceResources->GetSceneRenderTargetView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set the viewport.
    auto viewport = DeviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    DeviceResources->PIXEndEvent();
}

void App::RunBenchmark()
{
    FLog::Get().Log("Running benchmark");

    for(int sim = 0; sim < static_cast<int>(ENBodySim::NumSims); ++sim)
    {
        LARGE_INTEGER startTime, endTime;
        const int numFrames = 10;
        
        auto nbodySim = CreateNBodySim(DeviceResources->GetD3DDeviceContext(), static_cast<ENBodySim>(sim));
        nbodySim->Init(Particles);

        QueryPerformanceCounter(&startTime);

        for(int frame = 0; frame < numFrames; ++frame)
            nbodySim->Update(1.0f);

        QueryPerformanceCounter(&endTime);
        
        EventStream::Report(EEvent::BenchmarkResult, BenchmarkEventData(
            static_cast<ENBodySim>(sim),
            static_cast<int>((endTime.QuadPart - startTime.QuadPart) / (numFrames * 10000)))
        );

        Render();
    }

    FLog::Get().Log("Benchmark finished");
}

void App::CheckParticleSelected(DirectX::Mouse::State& ms)
{
    DirectX::SimpleMath::Vector2 mouse(static_cast<float>(ms.x),
                                       static_cast<float>(ms.y));

    bool found = false;

    for(auto& particle : Particles)
    {
        int x, y;

        if(Camera->PixelFromWorldPoint(particle.Position, x, y))
        {
            DirectX::SimpleMath::Vector2 screenPos(static_cast<float>(x), static_cast<float>(y));
            
            if(!found && DirectX::SimpleMath::Vector2::DistanceSquared(mouse, screenPos) < 100.0f)
            {
                particle.Colour = DirectX::Colors::Aqua;
                found = true;
                
                if(ms.leftButton)
                {
                    SelectedParticle = &particle;
                    UI->SetSelectedParticle(SelectedParticle);
                }
            }
            else
            {
                particle.Colour = particle.OriginalColour;
            }
        }
    }
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void App::OnActivated()
{
    // TODO: App is becoming active window.
}

void App::OnDeactivated()
{
    // TODO: App is becoming background window.
}

void App::OnSuspending()
{
    // TODO: App is being power-suspended (or minimized).
}

void App::OnResuming()
{
    Timer.ResetElapsedTime();

    // TODO: App is being power-resumed (or returning from minimize).
}

void App::OnWindowMoved()
{
    auto r = DeviceResources->GetOutputSize();
    DeviceResources->WindowSizeChanged(r.right, r.bottom);
}

void App::OnWindowSizeChanged(int width, int height)
{
    if (!DeviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void App::GetDefaultSize(int& width, int& height) const
{
    width = 1400;
    height = 900;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void App::CreateDeviceDependentResources()
{
    auto device = DeviceResources->GetD3DDevice();
    auto context = DeviceResources->GetD3DDeviceContext();

    Sim = CreateNBodySim(context, ENBodySim::BarnesHut);
    UI = std::make_unique<CUI>(context, DeviceResources->GetWindow());
    CommonStates = std::make_unique<DirectX::CommonStates>(device);

    ID3DBlob* VertexCode;

    bool bVertex = LoadVertexShader(device, L"shaders/PassThruGS.vsh", VertexShader.ReleaseAndGetAddressOf(), &VertexCode);
    bool bGeometry = LoadGeometryShader(device, L"shaders/DrawParticle.gsh", GeometryShader.ReleaseAndGetAddressOf());
    bool bPixel = LoadPixelShader(device, L"shaders/PlainColour.psh", PixelShader.ReleaseAndGetAddressOf());

    if(!(bVertex && bGeometry && bPixel))
        throw std::exception("Failed to load shader(s)");

    D3D11_INPUT_ELEMENT_DESC Layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    DX::ThrowIfFailed(
        device->CreateInputLayout(Layout, 2, VertexCode->GetBufferPointer(), VertexCode->GetBufferSize(), InputLayout.ReleaseAndGetAddressOf())
    );

    GSBuffer = std::make_unique<ConstantBuffer<Buffers::GS>>(device);

    InitParticles();
}

// Allocate all memory resources that change on a window SizeChanged event.
void App::CreateWindowSizeDependentResources()
{
    int width, height;
    GetDefaultSize(width, height);

    Camera = std::make_unique<CCamera>(width, height);

    PostProcess = std::make_unique<CPostProcess>(DeviceResources->GetD3DDevice(),
                                                  DeviceResources->GetD3DDeviceContext(),
                                                  width, height);

    Splatting = std::make_unique<CSplatting>(DeviceResources->GetD3DDeviceContext(), width, height);
}

void App::OnDeviceLost()
{
    
}

void App::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion
