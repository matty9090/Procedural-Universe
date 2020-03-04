#include "StarSystemSeeder.hpp"
#include "Physics.hpp"
//#include "Core/Event.hpp"

#include <random>
#include <DirectXColors.h>

#define _USE_MATH_DEFINES
#include <math.h>

template <class T>
StarSystemSeeder<T>::StarSystemSeeder(std::vector<T>& particles, float scale) : Particles(particles)
{

}

template <class T>
void StarSystemSeeder<T>::Seed(uint64_t seed)
{
    T& star = Particles[0];
    star.Position = DirectX::SimpleMath::Vector3::Zero;

    AddParticleVelocity(star, Vec3d());
    AddParticleMass(star, 1e30);
    AddParticleColour(star, DirectX::SimpleMath::Color(0.6f, 1.0f, 1.0f));
    AddParticleOriginalColour(star, star.Colour);
    AddParticleForces(star, Vec3d());
    AddParticleScale(star, 1.0f);

    std::default_random_engine generator;
    std::uniform_real_distribution<float>  dist_col(0.2f, 1.0f);
    std::uniform_real_distribution<float>  dist_red(0.0f, 0.4f);
    std::uniform_real_distribution<double> dist_rad(4.0 * Phys::AU * Phys::M, 7.0 * Phys::AU * Phys::M);
    std::uniform_real_distribution<double> dist_vel(1 * Phys::AU * Phys::M, 5 * Phys::AU * Phys::M);
    std::uniform_real_distribution<double> dist_mass(1e10, 1e26);
    std::uniform_real_distribution<double> dist_angle(0, 360.0f);

    for(int i = 1; i < Particles.size(); ++i)
    {
        Particles[i].Position.x = 0;
        Particles[i].Position.y = 0;
        Particles[i].Position.z = static_cast<float>(dist_rad(generator) / Phys::StarSystemScale);

        Vec3d vel(dist_vel(generator), dist_vel(generator) / 20.0, 0);

        AddParticleVelocity(Particles[i], vel);
        AddParticleMass(Particles[i], dist_mass(generator));
        AddParticleColour(Particles[i], DirectX::SimpleMath::Color(dist_red(generator), dist_col(generator), dist_col(generator), 1.0f));
        AddParticleOriginalColour(Particles[i], Particles[i].Colour);
        AddParticleForces(Particles[i], Vec3d());
        AddParticleScale(Particles[i], 1.0f);
    }

    //EventStream::Report(EEvent::TrackParticle, ParticleEventData(&Particles[0]));
}