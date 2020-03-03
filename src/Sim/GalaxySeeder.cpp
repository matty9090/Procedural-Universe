#include "GalaxySeeder.hpp"
#include "Core/Maths.hpp"
#include "Services/Log.hpp"

#include <DirectXColors.h>

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Color;

template <class T>
GalaxySeeder<T>::GalaxySeeder(std::vector<T>& particles, float scale)
    : Particles(particles),
      DistZ(0.0f, 16.0f),
      DistR(0.0f, 1.0f),
      DistG(0.0f, 1.0f),
      DistB(0.0f, 1.0f),
      DistMass(1e28, 1e30),
      Scale(scale)
{

}

template <class T>
void GalaxySeeder<T>::SetRedDist(float low, float hi)
{
    DistR = std::uniform_real_distribution<float>(Maths::Clamp(low, 0.0f, 1.0f), Maths::Clamp(hi, 0.0f, 1.0f));
}

template <class T>
void GalaxySeeder<T>::SetGreenDist(float low, float hi)
{
    DistG = std::uniform_real_distribution<float>(Maths::Clamp(low, 0.0f, 1.0f), Maths::Clamp(hi, 0.0f, 1.0f));
}

template <class T>
void GalaxySeeder<T>::SetBlueDist(float low, float hi)
{
    DistB = std::uniform_real_distribution<float>(Maths::Clamp(low, 0.0f, 1.0f), Maths::Clamp(hi, 0.0f, 1.0f));
}

template <class T>
void GalaxySeeder<T>::Seed(uint64_t seed)
{
    Gen = std::default_random_engine { static_cast<unsigned int>(seed) };

    LocalNum = 0;

    std::uniform_real_distribution<float> dist_pos(-2000.0f, 2000.0f);
    std::uniform_real_distribution<double> dist_vel(0.8, 1.2);

    CreateSpiralArm(0.0f, ArmPDist / 2);
    CreateSpiralArm(3.14f, ArmPDist / 2);

    for(int i = LocalNum; i < Particles.size(); ++i)
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

template <class T>
bool GalaxySeeder<T>::AddParticle(
    DirectX::SimpleMath::Vector3 Pos,
    Vec3<double> Vel,
    double Mass
)
{
    if (LocalNum < Particles.size())
    {
        Particles[LocalNum].Position = Pos / Scale;
        Particles[LocalNum].Velocity = Vel;
        Particles[LocalNum].Mass = Mass;
        Particles[LocalNum].Colour = Color(DistR(Gen), DistG(Gen), DistB(Gen));
        Particles[LocalNum].OriginalColour = Particles[LocalNum].Colour;
        Particles[LocalNum].Forces = Vec3d();

        ++LocalNum;

        return true;
    }

    return false;
}

template <class T>
void GalaxySeeder<T>::CreateSpiralArm(float offset, float dist)
{
    std::normal_distribution<float> distx(0.5f, 0.2f);
    std::uniform_real_distribution<float> disty(0.2f, 0.5f);

    float maxAngle = 6.0f;
    int loops = static_cast<int>(floor(maxAngle / 0.1f));
    float numPerLoop = floor((static_cast<float>(Particles.size()) * dist) / loops);

    for (float angle = 0.0f, r = 2.0f; angle < 6.0f; angle += 0.1f, r += 7.2f)
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
        auto sy = spiral - normal  * 400.0f;
        auto ey = spiral + normal  * 400.0f;
        
        for (int i = 0; i < numPerLoop; ++i)
        {
            auto position = Vector3::Lerp(sx, ex, distx(Gen)) + Vector3::Lerp(sy, ey, disty(Gen));
            auto velocity = normal * 2e16f * (1000.0f / mag);
            position.z = static_cast<float>(DistZ(Gen));

            AddParticle(position, Vec3d(velocity.x, velocity.y, velocity.z));
        }
    }
}