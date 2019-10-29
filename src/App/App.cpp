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
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void App::Initialize(HWND window, int width, int height)
{
    FLog::Get().Log("Initializing...");

    m_seeder = CreateParticleSeeder(m_particles, EParticleSeeder::Random);

    m_mouse = std::make_unique<DirectX::Mouse>();
    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60.0);

    RegisterEvents();

    FLog::Get().Log("Initialized");
}

void App::RegisterEvents()
{
    EventStream::Register(EEvent::SimSpeedChanged, [this](const EventData& data) {
        m_simSpeed = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::NumParticlesChanged, [this](const EventData& data) {
        m_numParticles = static_cast<const IntEventData&>(data).Value;
        InitParticles();
    });

    EventStream::Register(EEvent::SimTypeChanged, [this](const EventData& data) {
        m_sim.reset();
        m_sim = CreateNBodySim(m_deviceResources->GetD3DDeviceContext(), static_cast<const SimTypeEventData&>(data).Value);
        m_sim->Init(m_particles);
    });

    EventStream::Register(EEvent::IsPausedChanged, [this](const EventData& data) {
        m_isPaused = static_cast<const BoolEventData&>(data).Value;
    });

    EventStream::Register(EEvent::SeederChanged, [this](const EventData& data) {
        m_seeder = CreateParticleSeeder(m_particles, static_cast<const SeederTypeEventData&>(data).Value);
        InitParticles();
    });

    EventStream::Register(EEvent::ForceFrame, [this](const EventData& data) {
        float dt = static_cast<const FloatEventData&>(data).Value;
        m_sim->Update(dt * m_simSpeed);
    });

    EventStream::Register(EEvent::RunBenchmark, [this](const EventData& data) {
        RunBenchmark();
    });

    EventStream::Register(EEvent::DrawDebugChanged, [this](const EventData& data) {
        m_drawDebug = static_cast<const BoolEventData&>(data).Value;
    });

    EventStream::Register(EEvent::TrackParticle, [this](const EventData& data) {
        auto p = static_cast<const ParticleEventData&>(data).Value;
        m_camera->Track(p);
        FLog::Get().Log("Tracking particle");
    });

    EventStream::Register(EEvent::LoadParticleFile, [this](const EventData& data) {
        if (InitParticlesFromFile(static_cast<const StringEventData&>(data).Value, m_particles))
        {
            m_numParticles = static_cast<unsigned int>(m_particles.size());

            m_sim->Init(m_particles);
            m_particleBuffer.Reset();

            D3D11_BUFFER_DESC buffer;
            buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            buffer.Usage = D3D11_USAGE_DEFAULT;
            buffer.ByteWidth = m_numParticles * sizeof(Particle);
            buffer.CPUAccessFlags = 0;
            buffer.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA init;
            init.pSysMem = &m_particles[0];

            DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&buffer, &init, m_particleBuffer.ReleaseAndGetAddressOf()));
        }
    });

    EventStream::Register(EEvent::UseSplattingChanged, [this](const EventData& data) {
        m_useSplatting = static_cast<const BoolEventData&>(data).Value;
    });
}

void App::InitParticles()
{
    m_ui->SetSelectedParticle(nullptr);
    m_particles.resize(m_numParticles);
    m_seeder->Seed();
    m_sim->Init(m_particles);
    m_particleBuffer.Reset();

    CreateParticleBuffer(m_deviceResources->GetD3DDevice(), m_particleBuffer.ReleaseAndGetAddressOf(), m_particles);
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
    m_timer.Tick([&]()
    {
        Update(static_cast<float>(m_timer.GetElapsedSeconds()));
    });

    Render();
}

// Updates the world.
void App::Update(float dt)
{
    auto mouse_state = m_mouse->GetState();

    CheckParticleSelected(mouse_state);

    m_camera->Events(m_mouse.get(), mouse_state, dt);
    m_camera->Update(dt);
    m_ui->Update(dt);

    if(!m_isPaused)
        m_sim->Update(dt * m_simSpeed);

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->UpdateSubresource(m_particleBuffer.Get(), 0, NULL, &m_particles[0], 0, 0);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void App::Render()
{
    if (m_timer.GetFrameCount() == 0)
        return;

    m_deviceResources->PIXBeginEvent(L"Render");

    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    auto sceneTarget = m_deviceResources->GetSceneRenderTargetView();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto dsv = m_deviceResources->GetDepthStencilView();

    context->OMSetRenderTargets(1, &sceneTarget, dsv);
    
    // HACK
    RenderParticles();

    if(m_useSplatting)
        m_splatting->Render(m_numParticles, m_deviceResources->GetSceneShaderResourceView());
    
    RenderParticles();

    context->GSSetShader(nullptr, 0, 0);
    
    if(m_drawDebug)
        m_sim->RenderDebug(m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix());

    m_postProcess->Render(renderTarget, dsv, m_deviceResources->GetSceneShaderResourceView());

    m_ui->Render();

    m_deviceResources->PIXEndEvent();
    m_deviceResources->Present();
}

void App::RenderParticles()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    context->VSSetShader(m_vertexShader.Get(), 0, 0);
    context->GSSetShader(m_geometryShader.Get(), 0, 0);
    context->PSSetShader(m_pixelShader.Get(), 0, 0);

    unsigned int offset = 0;
    unsigned int stride = sizeof(Particle);

    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetVertexBuffers(0, 1, m_particleBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    
    DirectX::SimpleMath::Matrix view = m_camera->GetViewMatrix();
    DirectX::SimpleMath::Matrix proj = m_camera->GetProjectionMatrix();

    m_gsBuffer->SetData(context, Buffers::GS { view * proj, view.Invert() });
    context->GSSetConstantBuffers(0, 1, m_gsBuffer->GetBuffer());
    context->RSSetState(m_deviceResources->GetRasterizerState());
    context->OMSetBlendState(m_commonStates->Additive(), Colors::Black, 0xFFFFFFFF);
    context->Draw(m_numParticles, 0);
}

// Helper method to clear the back buffers.
void App::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();

    auto depthStencil = m_deviceResources->GetDepthStencilView();
    auto renderTarget = m_deviceResources->GetSceneRenderTargetView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void App::RunBenchmark()
{
    FLog::Get().Log("Running benchmark");

    for(int sim = 0; sim < static_cast<int>(ENBodySim::NumSims); ++sim)
    {
        LARGE_INTEGER startTime, endTime;
        const int numFrames = 10;
        
        auto nbodySim = CreateNBodySim(m_deviceResources->GetD3DDeviceContext(), static_cast<ENBodySim>(sim));
        nbodySim->Init(m_particles);

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

    for(auto& particle : m_particles)
    {
        int x, y;

        if(m_camera->PixelFromWorldPoint(particle.Position, x, y))
        {
            DirectX::SimpleMath::Vector2 screenPos(static_cast<float>(x), static_cast<float>(y));
            
            if(!found && DirectX::SimpleMath::Vector2::DistanceSquared(mouse, screenPos) < 100.0f)
            {
                particle.Colour = DirectX::Colors::Aqua;
                found = true;
                
                if(ms.leftButton)
                {
                    m_selectedParticle = &particle;
                    m_ui->SetSelectedParticle(m_selectedParticle);
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
    m_timer.ResetElapsedTime();

    // TODO: App is being power-resumed (or returning from minimize).
}

void App::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void App::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
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
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_sim = CreateNBodySim(context, ENBodySim::BarnesHut);
    m_ui = std::make_unique<UI>(context, m_deviceResources->GetWindow());
    m_commonStates = std::make_unique<DirectX::CommonStates>(device);

    ID3DBlob* VertexCode;

    bool bVertex = LoadVertexShader(device, L"shaders/PassThruGS.vsh", m_vertexShader.ReleaseAndGetAddressOf(), &VertexCode);
    bool bGeometry = LoadGeometryShader(device, L"shaders/DrawParticle.gsh", m_geometryShader.ReleaseAndGetAddressOf());
    bool bPixel = LoadPixelShader(device, L"shaders/PlainColour.psh", m_pixelShader.ReleaseAndGetAddressOf());

    if(!(bVertex && bGeometry && bPixel))
        throw std::exception("Failed to load shader(s)");

    D3D11_INPUT_ELEMENT_DESC Layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    DX::ThrowIfFailed(
        device->CreateInputLayout(Layout, 2, VertexCode->GetBufferPointer(), VertexCode->GetBufferSize(), m_inputLayout.ReleaseAndGetAddressOf())
    );

    m_gsBuffer = std::make_unique<ConstantBuffer<Buffers::GS>>(device);

    InitParticles();
}

// Allocate all memory resources that change on a window SizeChanged event.
void App::CreateWindowSizeDependentResources()
{
    int width, height;
    GetDefaultSize(width, height);

    m_camera = std::make_unique<Camera>(width, height);

    m_postProcess = std::make_unique<PostProcess>(m_deviceResources->GetD3DDevice(),
                                                  m_deviceResources->GetD3DDeviceContext(),
                                                  width, height);

    m_splatting = std::make_unique<Splatting>(m_deviceResources->GetD3DDeviceContext(), width, height);
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
