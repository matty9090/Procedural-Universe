#pragma once

#include "Render/Misc/Particle.hpp"

using namespace DirectX::SimpleMath;

namespace Phys
{
    const double G = 6.674e-11; // Newton's gravitational constant
    const double S = 1e1; // Softener
    const double M = 1000; // Meter (in this programs's unit)

    const double AU = 1.15e12; // Astronomical units
    const double LY = 9.46e15; // Light year

    const double StarSystemScale = 20 * AU;
    const double GalaxyScale = 1e6 * LY;

    const char GStr[] = "6.674e-11";
    const char SStr[] = "1e4";

    const char StarSystemScaleStr[] = "2.3e13";
    const char GalaxyScaleStr[] = "9.46e21";

    inline double Gravity(const Particle& a, const Particle& b)
    {
        Vec3d ap(a.Position.x, a.Position.y, a.Position.z);
        Vec3d bp(b.Position.x, b.Position.y, b.Position.z);

        ap *= StarSystemScale; 
        bp *= StarSystemScale;

        double d = Vector3::DistanceSquared(a.Position, b.Position);
        return -(G * a.Mass * b.Mass) / (d + S);
    }

    inline double Gravity(const Particle& a, const Vector3& b, double Mass)
    {
        Vec3d ap(a.Position.x, a.Position.y, a.Position.z);
        Vec3d bp(b.x, b.y, b.z);

        ap *= StarSystemScale; 
        bp *= StarSystemScale;

        double d = Vector3::DistanceSquared(a.Position, b);
        return -(G * a.Mass * Mass) / (d + S);
    }
}