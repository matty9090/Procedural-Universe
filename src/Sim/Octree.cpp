#include "Octree.hpp"

Octree::Octree(const Cube& bounds)
    : Bounds(bounds),
      Size((bounds.TopLeft - bounds.BottomRight).length())
{
    for(int i = 0; i < 8; ++i)
        Children[i] = nullptr;
}

void Octree::Split()
{
    float size = Size / 2;

    Vec3<> cur = Bounds.TopLeft;
    Vec3<> off(size, size, size);

    int i = 0;

    for(size_t z = 0; z < 2; ++z)
    {
        for(size_t y = 0; y < 2; ++y)
        {
            for(size_t x = 0; x < 2; ++x, ++i)
            {
                Cube bounds;
                bounds.TopLeft = cur;
                bounds.BottomRight = cur + off;

                Children[i] = std::make_unique<Octree>(bounds);

                cur.x += size;
            }

            cur.y += size;
        }

        cur.z += size;
    }
}