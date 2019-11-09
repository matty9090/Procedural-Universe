#pragma once

#include <memory>
#include <array>
#include <list>
#include <mutex>
#include <GeometricPrimitive.h>

#include "Core/Vec3.hpp"
#include "Render/Cube.hpp"
#include "Render/Particle.hpp"

struct BoundingCube
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
        Octree(const BoundingCube& bounds, int depth = 0);

        void Split();
        void Add(Particle* p);
        void CalculateMass();
        Vec3d CalculateForce(Particle *p);
        void RenderDebug(Cube* cube, DirectX::GeometricPrimitive* sphere, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

        std::mutex Mutex;
        int Depth = 0;
        int NumParticles = 0;
        BoundingCube Bounds;
        double TotalMass = 0.0;
        DirectX::SimpleMath::Vector3 CentreOfMass;

        static double Theta;

    private:
        bool IsLeaf = true;
        float Size = 0.0f;

        Particle* P = nullptr;
        std::array<std::unique_ptr<Octree>, 8> Children;
};