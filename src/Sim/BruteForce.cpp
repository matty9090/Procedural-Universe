#include "BruteForce.hpp"

void BruteForce::Update(float dt)
{
    for(auto& particle : Particles)
    {
        particle.Position.z -= dt * 40.0f;
    }
}