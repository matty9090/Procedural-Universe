#include "BruteForceCPU.hpp"
#include "Physics.hpp"
#include "Core/Vec3.hpp"
#include "Services/Log.hpp"
#include "Render/Shader.hpp"

#include <stdlib.h>
#include <thread>
#include <array>

using namespace DirectX::SimpleMath;

BruteForceCPU::BruteForceCPU(ID3D11DeviceContext* context)
    : Context(context)
{
    FLog::Get().Log("Brute Force CPU");
}

void BruteForceCPU::Init(std::vector<Particle>& particles)
{
    Particles = &particles;
}

void BruteForceCPU::Exec(size_t index, size_t loops)
{
    for(size_t i = index * loops; i < (index + 1) * loops; ++i)
    {
        for(size_t j = 0; j < Particles->size(); ++j)
        {
            if(i == j) continue;
            
            auto& a = (*Particles)[i];
            auto& b = (*Particles)[j];

            auto diff = (b.Position - a.Position);
            auto len = diff.Length();

            double f = Phys::Gravity(a, b);

            b.Forces += Vec3d(f * diff.x / len, f * diff.y / len, f * diff.z / len);
        }
    }
}

void BruteForceCPU::Update(float dt)
{
    const size_t numThreads = 8;
    std::array<std::thread, numThreads> threads;

    const size_t loopsPerThread = Particles->size() / numThreads;
    const int remainder = Particles->size() % numThreads;

    size_t thread;

    for(thread = 0; thread < numThreads; ++thread)
        threads[thread] = std::thread(&BruteForceCPU::Exec, this, thread, loopsPerThread);

    if(remainder > 0)
        Exec(thread, remainder);

    for(int thread = 0; thread < numThreads; ++thread)
        threads[thread].join();

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