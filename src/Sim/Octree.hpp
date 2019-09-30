#pragma once

#include <memory>
#include <array>

#include "Core/Vec3.hpp"

struct Cube
{
    Vec3<> TopLeft;
    Vec3<> BottomRight;    
};

class Octree
{
    public:
        Octree(const Cube& bounds);

        void Split();

    private:
        Cube Bounds;
        float Size;

        std::array<std::unique_ptr<Octree>, 8> Children;
};