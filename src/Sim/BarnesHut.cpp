#include "BarnesHut.hpp"
#include "Services/Log.hpp"
#include "Sim/Physics.hpp"

BarnesHut::BarnesHut(ID3D11DeviceContext* context) : Context(context)
{
    FLog::Get().Log("Barnes-Hut");

    const float size = 2000.0f;

    Bounds = {
        { -size, -size, -size },
        { +size, +size, +size }
    };

    Tree = std::make_unique<Octree>(Bounds);
    
    DebugCube = DirectX::GeometricPrimitive::CreateCube(context);
    DebugSphere = DirectX::GeometricPrimitive::CreateSphere(context);
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

    Tree->CalculateMass();
    particle = Particles->data();

    for(int i = 0; i < Particles->size(); ++i)
    {
        particle->Forces = Tree->CalculateForce(particle);
        particle++;
    }

    particle = Particles->data();

    for(int i = 0; i < Particles->size(); ++i)
    {
        auto a = particle->Forces / particle->Mass;
        particle->Velocity += a * dt;

        auto vel = (particle->Velocity * dt) / Phys::StarSystemScale;

        particle->Position += DirectX::SimpleMath::Vector3(static_cast<float>(vel.x),
                                                           static_cast<float>(vel.y),
                                                           static_cast<float>(vel.z));
                                                           
        particle++;
    }
}

void BarnesHut::RenderDebug(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
    Tree->RenderDebug(DebugCube.get(), DebugSphere.get(), view, proj);
}