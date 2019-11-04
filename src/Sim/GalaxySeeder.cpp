#include "GalaxySeeder.hpp"
#include <DirectXColors.h>

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Color;

GalaxySeeder::GalaxySeeder(std::vector<Particle>& particles)
    : Particles(particles),
      DistZ(0.0f, 16.0f),
      DistCol(0.2f, 1.0f),
      DistRed(0.0f, 0.3f),
      DistMass(1e28, 1e30)
{

}

void GalaxySeeder::Seed()
{
    LocalNum = 0;

    std::uniform_real_distribution<float> dist_pos(-1000.0f, 1000.0f);
    std::uniform_real_distribution<double> dist_vel(0.8, 1.2);

    CreateSpiralArm(0.0f, ArmPDist / 2);
    CreateSpiralArm(3.14f, ArmPDist / 2);

    AddParticle(Centre, Vec3d(), 1e32);

    for(int i = 0; i < Particles.size() - LocalNum; ++i)
    {
        Vector3 pos;
        pos.x = static_cast<float>(dist_pos(Gen));
        pos.y = static_cast<float>(dist_pos(Gen));
        pos.z = static_cast<float>(DistZ(Gen));

        if (Vector3::Distance(pos, Centre) > 720.0f)
        {
            --i;
            continue;
        }

        Vector3 norm = pos - Centre;
        Vector3 tangent = norm.Cross(Vector3(0.0f, 0.0f, 1.0f));

        Vec3d vel(tangent.x, tangent.y, tangent.z);
        vel *= dist_vel(Gen) * 1e14;

        if (!AddParticle(pos, vel, DistMass(Gen)))
            break;
    }
}

bool GalaxySeeder::AddParticle(
    DirectX::SimpleMath::Vector3 Pos,
    Vec3<double> Vel,
    double Mass
)
{
    if(LocalNum < Particles.size())
    {
        Particles[LocalNum].Position = Pos;
        Particles[LocalNum].Velocity = Vel;
        Particles[LocalNum].Mass = Mass;
        Particles[LocalNum].Colour = Color(DistRed(Gen), DistCol(Gen), DistCol(Gen));
        Particles[LocalNum].OriginalColour = Particles[LocalNum].Colour;
        Particles[LocalNum].Forces = Vec3d();

        ++LocalNum;

        return true;
    }

    return false;
}

void GalaxySeeder::CreateSpiralArm(float offset, float dist)
{
    std::normal_distribution<float> distx(0.5f, 0.2f);
    std::uniform_real_distribution<float> disty(0.5f, 0.2f);

    float maxAngle = 6.0f;
    int loops = static_cast<int>(floor(maxAngle / 0.1f));
    float numPerLoop = floor((static_cast<float>(Particles.size()) * dist) / loops);

    for(float angle = 0.0f, r = 2.0f; angle < 6.0f; angle += 0.1f, r += 7.2f)
    {
        auto spiral = Vector3(cosf(angle + offset) * r, sinf(angle + offset) * r, 0.0f);
        auto spiraln = Vector3(cosf(angle + offset + 0.1f) * (r + 10.0f), sinf(angle + offset + 0.1f) * (r + 10.0f), 0.0f);
        
        auto normal = (spiral - spiraln);
        auto mag = normal.Length();
        normal.Normalize();

        Vector3 tangent = normal.Cross(Vector3(0.0f, 0.0f, 1.0f));
        tangent.Normalize();

        auto sx = spiral - tangent * 140.0f;
        auto ex = spiral + tangent * 140.0f;
        auto sy = spiral - normal  * 100.0f;
        auto ey = spiral + normal  * 100.0f;
        
        for (int i = 0; i < numPerLoop; ++i)
        {
            auto position = Vector3::Lerp(sx, ex, distx(Gen)) + Vector3::Lerp(sy, ey, disty(Gen));
            auto velocity = normal * 2e16f * (1000.0f / mag);
            position.z = static_cast<float>(DistZ(Gen));

            AddParticle(position, Vec3d(velocity.x, velocity.y, velocity.z));
        }
    }
}