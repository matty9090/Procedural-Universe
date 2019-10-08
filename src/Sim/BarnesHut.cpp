#include "BarnesHut.hpp"
#include "Services/Log.hpp"
#include "Sim/Physics.hpp"

#include <thread>

BarnesHut::BarnesHut(ID3D11DeviceContext* context) : Context(context)
{
    FLog::Get().Log("Barnes-Hut");

    const float size = 4000.0f;

    Bounds = {
        { -size, -size, -size },
        { +size, +size, +size }
    };

    Tree = std::make_unique<Octree>(Bounds);
    
    if(context)
    {
        DebugCube = DirectX::GeometricPrimitive::CreateCube(context);
        DebugSphere = DirectX::GeometricPrimitive::CreateSphere(context);
    }
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

    const size_t numThreads = 8;
    std::array<std::thread, numThreads> threads;

    const size_t loopsPerThread = Particles->size() / numThreads;
    const int remainder = Particles->size() % numThreads;

    size_t thread;

    for(thread = 0; thread < numThreads; ++thread)
    {
        threads[thread] = std::thread([&](size_t t, size_t loops) {
            for(size_t i = t * loops; i < (t + 1) * loops; ++i)
            {
                Particle* p = &(*Particles)[i];
                p->Forces = Tree->CalculateForce(p);
            }
        }, thread, loopsPerThread);
    }
    
    if(remainder > 0)
    {
        for(size_t i = thread; i < thread + remainder; ++i)
        {
            Particle* p = &(*Particles)[i];
            p->Forces = Tree->CalculateForce(p);
        }
    }

    for(int thread = 0; thread < numThreads; ++thread)
        threads[thread].join();

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