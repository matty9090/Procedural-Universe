#include "BruteForceCPU.hpp"
#include "Physics.hpp"
#include "Core/Vec3.hpp"
#include "Services/Log.hpp"
#include "Render/Shader.hpp"

#include <stdlib.h>

using namespace DirectX::SimpleMath;

BruteForceCPU::BruteForceCPU(ID3D11DeviceContext* context)
    : Context(context)
{
    FLog::Get().Log("Brute Force CPU");
}

void BruteForceCPU::Init(std::vector<Particle>& particles)
{
    Particles = &particles;
    Accel.resize(Particles->size());
}

void BruteForceCPU::Exec(float dt, size_t index, size_t loops)
{
    for(size_t i = index * loops; i < (index + 1) * loops; ++i)
    {
        auto& a = (*Particles)[i];

        for(size_t j = i + 1; j < Particles->size(); ++j)
        {
            auto& b = (*Particles)[j];

            auto diff = a.Position - b.Position;
            auto dist = diff.Length();
            auto d3   = dist * dist * dist;

            float fa = dt * a.Mass / d3;
            float fb = dt * b.Mass / d3;

            Accel[i] -= Vector3(fa * diff.x, fa * diff.y, fa * diff.z);
            Accel[j] += Vector3(fb * diff.x, fb * diff.y, fb * diff.z);
        }
    }
}

void BruteForceCPU::Update(float dt)
{
    for(int i = 0; i < Particles->size(); ++i)
        Accel[i] = Vector3::Zero;

    const size_t numThreads = 8;
    const size_t loopsPerThread = Particles->size() / numThreads;
    const int remainder = Particles->size() % numThreads;

    size_t thread;

    Threads.clear();

    for(thread = 0; thread < numThreads; ++thread)
        Threads.push_back(std::thread(&BruteForceCPU::Exec, this, dt, thread, loopsPerThread));

    if(remainder > 0)
        Threads.push_back(std::thread(&BruteForceCPU::Exec, this, dt, thread, remainder));

    for(auto& thread : Threads)
        thread.join();

    for(int i = 0; i < Particles->size(); ++i)
    {
        auto& p = (*Particles)[i];
        p.Velocity += Accel[i] * dt;

        p.Position += Vector3(static_cast<float>(p.Velocity.x),
                              static_cast<float>(p.Velocity.y),
                              static_cast<float>(p.Velocity.z));
    }
}