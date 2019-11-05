#include "BarnesHut.hpp"
#include "Services/Log.hpp"
#include "Sim/Physics.hpp"
#include "Core/Event.hpp"

#include <thread>

BarnesHut::BarnesHut(ID3D11DeviceContext* context)
    : Context(context),
      Pool(std::bind(&BarnesHut::Exec, this, std::placeholders::_1))
{
    LOGM("Barnes-Hut")

    const float size = 4000.0f;

    Bounds = {
        { -size, -size, -size },
        { +size, +size, +size }
    };

    Tree = std::make_unique<Octree>(Bounds);
    
    if(context)
    {
        DebugCube = std::make_unique<Cube>(context);
        DebugSphere = DirectX::GeometricPrimitive::CreateSphere(context);
    }

    EventStream::Register(EEvent::BHThetaChanged, [&](const EventData& data) {
        Octree::Theta = static_cast<const FloatEventData&>(data).Value;
    });
}

BarnesHut::~BarnesHut()
{
    EventStream::UnregisterAll(EEvent::BHThetaChanged);
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

    const size_t numThreads = Pool.GetNumWorkers();
    const size_t loopsPerThread = Particles->size() / numThreads;
    const size_t remainder = Particles->size() % numThreads;

    int32_t thread;

    for(thread = 0; thread < numThreads; ++thread)
    {
        Pool.Dispatch(thread, { static_cast<size_t>(thread), loopsPerThread });
    }
    
    if(remainder > 0)
    {
        for(int32_t i = thread; i < thread + remainder; ++i)
        {
            Particle* p = &(*Particles)[i];
            p->Forces = Tree->CalculateForce(p);
        }
    }

    Pool.Join();

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

void BarnesHut::Exec(const ParticleInfo& info)
{
    for (size_t i = info.Index * info.Loops; i < (info.Index + 1) * info.Loops; ++i)
    {
        Particle* p = &(*Particles)[i];
        p->Forces = Tree->CalculateForce(p);
    }
}
