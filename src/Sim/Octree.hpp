#pragma once

#include <memory>
#include <array>
#include <list>
#include <GeometricPrimitive.h>

#include "Core/Vec3.hpp"
#include "Render/Particle.hpp"

struct Cube
{
    Vec3<> TopLeft;
    Vec3<> BottomRight;    

    bool Contains(Particle* p)
    {
        return p->Position.x >= TopLeft.x     && p->Position.y >= TopLeft.y     && p->Position.z >= TopLeft.z &&
               p->Position.x <  BottomRight.x && p->Position.y <  BottomRight.y && p->Position.z <  BottomRight.z;
    }
};

class Octree
{
    public:
        Octree(const Cube& bounds);

        void Split();
        void Add(Particle* p);

        void RenderDebug(DirectX::GeometricPrimitive* cube, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

        Cube Bounds;

    private:
        bool IsLeaf = true;
        float Size;
        const int MaxParticles = 10;

        std::list<Particle*> Particles;
        std::array<std::unique_ptr<Octree>, 8> Children;
};