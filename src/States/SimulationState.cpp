#include "SimulationState.hpp"
#include "Services/Log.hpp"

#include <fstream>
#include <direct.h>

void SimulationState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;

    Mouse = mouse;
    Seeder = CreateParticleSeeder(Particles, EParticleSeeder::Random);

    RegisterEvents();

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

void SimulationState::Cleanup()
{
    UI.reset();

    EventStream::UnregisterAll(EEvent::SimSpeedChanged);
    EventStream::UnregisterAll(EEvent::NumParticlesChanged);
    EventStream::UnregisterAll(EEvent::SimTypeChanged);
    EventStream::UnregisterAll(EEvent::IsPausedChanged);
    EventStream::UnregisterAll(EEvent::SeederChanged);
    EventStream::UnregisterAll(EEvent::ForceFrame);
    EventStream::UnregisterAll(EEvent::RunBenchmark);
    EventStream::UnregisterAll(EEvent::DrawDebugChanged);
    EventStream::UnregisterAll(EEvent::TrackParticle);
    EventStream::UnregisterAll(EEvent::LoadParticleFile);
    EventStream::UnregisterAll(EEvent::UseSplattingChanged);
    EventStream::UnregisterAll(EEvent::BenchmarkResult);
}

void SimulationState::Update(float dt)
{
    auto mouse_state = Mouse->GetState();

    CheckParticleSelected(mouse_state);

    Camera->Events(Mouse, mouse_state, dt);
    Camera->Update(dt);
    UI->Update(dt);

    if (!bIsPaused)
        Sim->Update(dt * SimSpeed);

    auto context = DeviceResources->GetD3DDeviceContext();
    context->UpdateSubresource(ParticleBuffer.Get(), 0, NULL, &Particles[0], 0, 0);
}

void SimulationState::Render()
{
    Clear();

    auto sceneTarget = DeviceResources->GetSceneRenderTargetView();
    auto renderTarget = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView();

    Context->OMSetRenderTargets(1, &sceneTarget, dsv);

    // HACK
    RenderParticles();

    if (bUseSplatting)
        Splatting->Render(NumParticles, DeviceResources->GetSceneShaderResourceView());

    RenderParticles();

    Context->GSSetShader(nullptr, 0, 0);

    if (bDrawDebug)
        Sim->RenderDebug(Camera->GetViewMatrix(), Camera->GetProjectionMatrix());

    PostProcess->Render(renderTarget, dsv, DeviceResources->GetSceneShaderResourceView());

    UI->Render();
}

void SimulationState::RenderParticles()
{
    DirectX::SimpleMath::Matrix view = Camera->GetViewMatrix();
    DirectX::SimpleMath::Matrix proj = Camera->GetProjectionMatrix();

    ParticlePipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Particle);

        Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
        GSBuffer->SetData(Context, Buffers::GS{ view * proj, view.Invert() });
        Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
        Context->RSSetState(DeviceResources->GetRasterizerState());
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);
        Context->Draw(NumParticles, 0);
    });
}

void SimulationState::CreateDeviceDependentResources()
{
    Sim = CreateNBodySim(Context, ENBodySim::BarnesHut);
    UI = std::make_unique<CUI>(Context, DeviceResources->GetWindow());
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);

    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(Device, L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(Device, L"shaders/PlainColour.psh");
    ParticlePipeline.LoadGeometry(Device, L"shaders/DrawParticle.gsh");

    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ParticlePipeline.CreateInputLayout(Device, Layout);

    GSBuffer = std::make_unique<ConstantBuffer<Buffers::GS>>(Device);

    InitParticles();
}

void SimulationState::CreateWindowSizeDependentResources()
{
    auto width = DeviceResources->GetScreenViewport().Width;
    auto height = DeviceResources->GetScreenViewport().Height;

    Camera = std::make_unique<CCamera>(width, height);

    PostProcess = std::make_unique<CPostProcess>(DeviceResources->GetD3DDevice(),
        DeviceResources->GetD3DDeviceContext(),
        width, height);

    Splatting = std::make_unique<CSplatting>(DeviceResources->GetD3DDeviceContext(), width, height);
}

void SimulationState::Clear()
{
    DeviceResources->PIXBeginEvent(L"Clear");

    auto context = DeviceResources->GetD3DDeviceContext();

    auto depthStencil = DeviceResources->GetDepthStencilView();
    auto renderTarget = DeviceResources->GetSceneRenderTargetView();

    context->ClearRenderTargetView(renderTarget, DirectX::Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto viewport = DeviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    DeviceResources->PIXEndEvent();
}

void SimulationState::RegisterEvents()
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

            CreateParticleBuffer(DeviceResources->GetD3DDevice(), ParticleBuffer.ReleaseAndGetAddressOf(), Particles);
        }
    });

    EventStream::Register(EEvent::UseSplattingChanged, [this](const EventData& data) {
        bUseSplatting = static_cast<const BoolEventData&>(data).Value;
    });
}

void SimulationState::InitParticles()
{
    UI->SetSelectedParticle(nullptr);
    Particles.resize(NumParticles);
    Seeder->Seed();
    Sim->Init(Particles);
    ParticleBuffer.Reset();

    CreateParticleBuffer(DeviceResources->GetD3DDevice(), ParticleBuffer.ReleaseAndGetAddressOf(), Particles);
}

bool SimulationState::InitParticlesFromFile(std::string fname, std::vector<Particle>& particles)
{
    std::ifstream infile("data/" + fname, std::ios::binary);

    if (!infile.is_open())
    {
        FLog::Get().Log("Could not read particle file " + fname, FLog::Error);
        return false;
    }

    particles.clear();

    while (true)
    {
        Particle p;
        infile.read(reinterpret_cast<char*>(&p), sizeof(Particle));

        if (!infile)
            break;

        particles.push_back(p);
    }

    infile.close();

    FLog::Get().Log("Read " + std::to_string(particles.size()) + " particles from file");

    long double TotalMass = 0.0;
    Vec3d CentreOfMass;

    for (const auto& particle : particles)
    {
        Vec3d pos(particle.Position.x, particle.Position.y, particle.Position.z);

        TotalMass += particle.Mass;
        CentreOfMass += pos * particle.Mass;
    }

    CentreOfMass /= TotalMass;

    DirectX::SimpleMath::Vector3 Centre(static_cast<float>(CentreOfMass.x),
        static_cast<float>(CentreOfMass.y),
        static_cast<float>(CentreOfMass.z));

    for (auto& particle : particles)
        particle.Position -= Centre;

    return true;
}

void SimulationState::RunSimulation(float dt, int time, int numparticles, std::string fileName)
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

    while (true)
    {
        ++iterations;

        QueryPerformanceCounter(&endTime);
        timer = endTime;

        if (static_cast<int>((endTime.QuadPart - startTime.QuadPart) / 10000000) >= time)
            break;

        if (static_cast<int>(((timer.QuadPart - tstart.QuadPart) / 10000000) >= 1))
        {
            QueryPerformanceCounter(&tstart);
            FLog::Get().Log("Running... (" + std::to_string(iterations) + " iterations)");
        }

        sim->Update(dt);
    }

    _mkdir("data");
    auto filename = "data/" + std::to_string(endTime.QuadPart) + ".nbody";
    std::ofstream file(filename, std::ios::binary);

    if (file.is_open())
    {
        for (const auto& p : particles)
        {
            file.write(reinterpret_cast<const char*>(&p), sizeof(p));
        }

        file.close();
    }
}

void SimulationState::RunBenchmark()
{
    FLog::Get().Log("Running benchmark");

    for (int sim = 0; sim < static_cast<int>(ENBodySim::NumSims); ++sim)
    {
        LARGE_INTEGER startTime, endTime;
        const int numFrames = 10;

        auto nbodySim = CreateNBodySim(DeviceResources->GetD3DDeviceContext(), static_cast<ENBodySim>(sim));
        nbodySim->Init(Particles);

        QueryPerformanceCounter(&startTime);

        for (int frame = 0; frame < numFrames; ++frame)
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

void SimulationState::CheckParticleSelected(DirectX::Mouse::State& ms)
{
    DirectX::SimpleMath::Vector2 mouse(static_cast<float>(ms.x),
        static_cast<float>(ms.y));

    bool found = false;

    for (auto& particle : Particles)
    {
        int x, y;

        if (Camera->PixelFromWorldPoint(particle.Position, x, y))
        {
            DirectX::SimpleMath::Vector2 screenPos(static_cast<float>(x), static_cast<float>(y));

            if (!found && DirectX::SimpleMath::Vector2::DistanceSquared(mouse, screenPos) < 100.0f)
            {
                particle.Colour = DirectX::Colors::Aqua;
                found = true;

                if (ms.leftButton)
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