#include "BarnesHut.hpp"
#include "Services/Log.hpp"

BarnesHut::BarnesHut(ID3D11DeviceContext* context) : Context(context)
{
    FLog::Get().Log("Barnes-Hut");

    const float size = 1000.0f;

    Bounds = {
        { -size, -size, -size },
        { +size, +size, +size }
    };

    Tree = std::make_unique<Octree>(Bounds);
    DebugCube = DirectX::GeometricPrimitive::CreateCube(context);
}

void BarnesHut::Init(std::vector<Particle>& particles)
{
    Particles = &particles;
}

void BarnesHut::Update(float dt)
{
    Tree = std::make_unique<Octree>(Bounds);

    Particle* particle = Particles->data();

    for(int i = 0; i < Particles->size(); ++i)
    {
        Tree->Add(particle);
        particle++;
    }
}

void BarnesHut::RenderDebug(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
    Tree->RenderDebug(DebugCube.get(), view, proj);
}