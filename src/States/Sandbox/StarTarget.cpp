#include "StarTarget.hpp"

#include <imgui.h>

#include "Sim/IParticleSeeder.hpp"
#include "Misc/Shapes.hpp"
#include "Misc/ProcUtils.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

StarTarget::StarTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Stellar", "Planet", resources, camera, rtv)
{
    Scale = 0.0005f;
    BeginTransitionDist = 10.0f;
    EndTransitionDist = 0.2f;
    RenderParentInChildSpace = true;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);

    CreateStarPipeline();
    CreateOrbitPipeline();
}

void StarTarget::Render()
{
    RenderParentSkybox();
    Parent->RenderInChildSpace(*Camera, 100.0f / Scale);

    RenderLerp();

    for (auto& planet : Planets)
    {
        planet->Render();
    }
}

void StarTarget::RenderObjectUI()
{
    ImGui::Text("Seed: %i", static_cast<int>(ParticleInfo[CurrentClosestObjectID].Seed));
    ImGui::Text("Radius: %i", static_cast<int>(ParticleInfo[CurrentClosestObjectID].Radius));
}

void StarTarget::RenderInChildSpace(const ICamera& cam, float scale)
{
    RenderParentSkybox();
    Parent->RenderInChildSpace(cam, scale);
    RenderLerp(Child->Scale, 0.0f, true);
}

void StarTarget::RenderTransitionChild(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Star->Move(ParentLocationSpace);
    RenderLerp(Scale, t, true);
    Star->Move(-ParentLocationSpace);

    for (auto& planet : Planets)
    {
        planet->Move(ParentLocationSpace);
        planet->Render(Scale, t);
        planet->Move(-ParentLocationSpace);
    }
}

void StarTarget::RenderTransitionParent(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    RenderParentSkybox();
    Parent->RenderInChildSpace(*Camera, 100.0f / Scale);
}

void StarTarget::MoveObjects(Vector3 v)
{
    for (auto& planet : Planets)
        planet->Move(v);

    Star->Move(v);
    Centre += v;

    ParentOffset += v;
    Parent->MoveObjects(v);
}

void StarTarget::ScaleObjects(float scale)
{
    for (auto& planet : Planets)
        planet->Scale(1.0f / scale);
}

Vector3 StarTarget::GetClosestObject(Vector3 pos)
{
    return Maths::ClosestParticle(pos, Particles, &CurrentClosestObjectID).Position;
}

Vector3 StarTarget::GetLightDirection() const
{
    auto pos = Particles[CurrentClosestObjectID].Position;
    pos.Normalize();
    return pos;
}

void StarTarget::OnStartTransitionDownParent(Vector3 object)
{
    GenerateSkybox(object);
}

void StarTarget::OnStartTransitionDownChild(Vector3 object)
{

}

void StarTarget::OnEndTransitionDownChild()
{

}

void StarTarget::RenderLerp(float scale, float t, bool single)
{
    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    if (single)
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });
    else
        LerpBuffer->SetData(Context, LerpConstantBuffer { t });
    
    Context->PSSetConstantBuffers(0, 1, LerpBuffer->GetBuffer());
    Context->GSSetShader(nullptr, 0, 0);
    Context->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
    Star->SetScale(scale);
    Star->Draw(Context, viewProj, StarPipeline);

    OrbitPipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Vertex);

        Context->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
        Context->IASetIndexBuffer(OrbitIB.Get(), DXGI_FORMAT_R16_UINT, 0);
        Context->VSSetConstantBuffers(0, 1, OrbitVCB->GetBuffer());

        for (auto& orbit : Orbits)
        {
            auto world = orbit.Orientation * Matrix::CreateTranslation(Centre) * Matrix::CreateScale(scale);
            orbit.Colour.A(t);
            OrbitVCB->SetData(Context, { world * viewProj, world });
            OrbitPCB->SetData(Context, { orbit.Colour });
            Context->PSSetConstantBuffers(0, 1, OrbitPCB->GetBuffer());
            Context->IASetVertexBuffers(0, 1, orbit.VertexBuffer.GetAddressOf(), &stride, &offset);
            Context->DrawIndexed(NumOrbitIndices, 0, 0);
        }
    });
}

void StarTarget::BakeSkybox(Vector3 object)
{
    SkyboxGenerator->Render([&](const ICamera& cam) {
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });

        Matrix view = cam.GetViewMatrix();
        Matrix viewProj = view * cam.GetProjectionMatrix();
        
        Parent->RenderInChildSpace(cam, 100.0f / Scale);

        for (auto& planet : Planets)
        {
            planet->Render();
        }

        Context->GSSetShader(nullptr, 0, 0);
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });
        Context->PSSetConstantBuffers(0, 1, LerpBuffer->GetBuffer());
        Context->GSSetShader(nullptr, 0, 0);
        Context->OMSetBlendState(CommonStates->AlphaBlend(), DirectX::Colors::Black, 0xFFFFFFFF);
        Star->Draw(Context, viewProj, StarPipeline);
    });
}

void StarTarget::Seed(uint64_t seed)
{
    auto gen = std::default_random_engine { static_cast<unsigned int>(seed) };
    std::uniform_int_distribution<> dist(4, 15);

    Particles.resize(dist(gen));
    ParticleInfo.resize(Particles.size());

    auto seeder = CreateParticleSeeder(Particles, EParticleSeeder::Random, 4.0f);
    seeder->Seed(seed);

    SeedQueue.clear();
    Orbits.clear();
    Planets.resize(Particles.size());

    for (size_t i = 0; i < Planets.size(); ++i)
    {
        auto pos = Particles[i].Position;

        Planets[i] = std::make_unique<CPlanet>(Context, *Camera);
        Planets[i]->SetPosition(pos);
        Planets[i]->Scale(Scale * 2.0f);
        Planets[i]->LightSource = -pos;
        Planets[i]->LightSource.Normalize();

        SeedQueue.push_back((seed << 5) + (Planets.size() - i - 1));
        
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        Vector3 axis = { pos.x, 0.0f, pos.z };
        axis = axis.Cross(Vector3::Up);
        axis.Normalize();

        float angleh = atan2f(pos.x, pos.z);
        float anglev = atanf(pos.y / Vector2(pos.x, pos.z).Length());

        auto rot = Quaternion::CreateFromAxisAngle(Vector3::Up, angleh);
        rot *= Quaternion::CreateFromAxisAngle(axis, anglev);

        Orbit orbit;
        orbit.Radius = pos.Length();
        orbit.Colour = ProcUtils::RandomColour(gen);
        orbit.Orientation = Matrix::CreateFromQuaternion(rot);
        
        Shapes::ComputeTorus(vertices, indices, orbit.Radius * 2.0f, 0.03f, 200);
        CreateVertexBuffer(Device, vertices, orbit.VertexBuffer.ReleaseAndGetAddressOf());

        Orbits.push_back(orbit);
    }
}

void StarTarget::ResetObjectPositions()
{
    MoveObjects(-Centre);
    Star->SetPosition(Vector3::Zero);
    Centre = Vector3::Zero;
}

void StarTarget::CreateStarPipeline()
{
    Star = std::make_unique<CModel>(Device, RESM.GetMesh("assets/Sphere.obj"));
    Star->Scale(0.004f);

    StarPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    StarPipeline.LoadVertex(L"shaders/Standard/Position.vsh");
    StarPipeline.LoadPixel(L"shaders/Particles/Star.psh");
    StarPipeline.CreateDepthState(Device, EDepthState::Normal);
    StarPipeline.CreateRasteriser(Device, ECullMode::Clockwise);
    StarPipeline.CreateInputLayout(Device, CreateInputLayoutPosition());

    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
}

void StarTarget::CreateOrbitPipeline()
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    Shapes::ComputeTorus(vertices, indices, BeginTransitionDist / 2.0f, 0.002f, 200);
    CreateIndexBuffer(Device, indices, OrbitIB.ReleaseAndGetAddressOf());

    OrbitPipeline.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    OrbitPipeline.LoadVertex(L"shaders/Standard/PositionNormalTexture.vsh");
    OrbitPipeline.LoadPixel(L"shaders/Planet/Ring.psh");
    OrbitPipeline.CreateDepthState(Device, EDepthState::Normal);
    OrbitPipeline.CreateInputLayout(Device, CreateInputLayoutPositionNormalTexture());

    NumOrbitIndices = static_cast<UINT>(indices.size());

    OrbitVCB = std::make_unique<ConstantBuffer<OrbitVSBuffer>>(Device);
    OrbitPCB = std::make_unique<ConstantBuffer<OrbitPSBuffer>>(Device);
}

void StarTarget::StateTransitioning(float dt)
{
    ++SeedFrames;

    if (!SeedQueue.empty() && SeedFrames > 10)
    {
        auto i = SeedQueue.size() - 1;

        CPlanetSeeder seeder(SeedQueue.front());
        seeder.SeedPlanet(Planets[i].get());
        ParticleInfo[i] = seeder;

        SeedQueue.pop_front();
        SeedFrames = 0;
    }
}
