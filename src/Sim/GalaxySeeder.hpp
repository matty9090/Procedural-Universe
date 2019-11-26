#pragma once

#include "IParticleSeeder.hpp"

#include <SimpleMath.h>
#include <random>

class GalaxySeeder : public IParticleSeeder
{
    public:
        GalaxySeeder(std::vector<Particle>& particles);

        void Seed(uint64_t seed);

    private:
        std::vector<Particle>& Particles;
        int LocalNum = 0;

        bool AddParticle(
            DirectX::SimpleMath::Vector3 Pos,
            Vec3d Vel = Vec3d(),
            double Mass = 1e20
        );

        void CreateSpiralArm(float offset, float dist);

        const float ArmPDist = 0.8f;
        
        std::normal_distribution<float> DistZ;
        std::uniform_real_distribution<float> DistCol;
        std::uniform_real_distribution<float> DistRed;
        std::uniform_real_distribution<double> DistMass;
        
        std::default_random_engine Gen;
        DirectX::SimpleMath::Vector3 Centre;
};