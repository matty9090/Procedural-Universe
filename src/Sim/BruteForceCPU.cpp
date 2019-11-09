#include "BruteForceCPU.hpp"
#include "Physics.hpp"
#include "Core/Vec3.hpp"
#include "Services/Log.hpp"
#include "Render/DX/Shader.hpp"

#include <stdlib.h>
#include <thread>
#include <array>

using namespace DirectX::SimpleMath;

BruteForceCPU::BruteForceCPU(ID3D11DeviceContext* context)
    : Context(context),
      Pool(std::bind(&BruteForceCPU::Exec, this, std::placeholders::_1))
{
    LOGM("Brute Force CPU")
}

void BruteForceCPU::Init(std::vector<Particle>& particles)
{
    Particles = &particles;
}

void BruteForceCPU::Exec(const ParticleInfo& info)
{
    for(size_t i = info.Index * info.Loops; i < (info.Index + 1) * info.Loops; ++i)
    {
        for(size_t j = 0; j < Particles->size(); ++j)
        {
            if(i == j) continue;
            
            auto& a = (*Particles)[j];
            auto& b = (*Particles)[i];

            auto diff = (b.Position - a.Position);
            diff.Normalize();

            double f = Phys::Gravity(a, b);
            b.Forces += Vec3d(f * diff.x, f * diff.y, f * diff.z);
        }
    }
}

void BruteForceCPU::Update(float dt)
{
    const size_t numThreads = Pool.GetNumWorkers();
    const size_t loopsPerThread = Particles->size() / numThreads;
    const size_t remainder = Particles->size() % numThreads;

    uint32_t thread;

    for (thread = 0; thread < numThreads; ++thread)
        Pool.Dispatch(thread, { thread, loopsPerThread });

    if(remainder > 0)
        Exec({ thread, remainder });

    Pool.Join();

    for(int i = 0; i < Particles->size(); ++i)
    {
        auto a = (*Particles)[i].Forces / (*Particles)[i].Mass;
        (*Particles)[i].Velocity += a * dt;

        auto vel = ((*Particles)[i].Velocity * dt) / Phys::StarSystemScale;

        (*Particles)[i].Position += Vector3(static_cast<float>(vel.x),
                                            static_cast<float>(vel.y),
                                            static_cast<float>(vel.z));
        
        (*Particles)[i].Forces = Vec3d();
    }
}