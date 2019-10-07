#include "Octree.hpp"
#include "Services/Log.hpp"

Octree::Octree(const Cube& bounds)
    : Bounds(bounds),
      Size(bounds.BottomRight.x - bounds.TopLeft.x)
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
        cur.y = Bounds.TopLeft.y;

        for(size_t y = 0; y < 2; ++y)
        {
            cur.x = Bounds.TopLeft.x;

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

    IsLeaf = false;
}

void Octree::Add(Particle* p)
{
    // Node has already split, add it to correct child node
    if(!IsLeaf)
    {
        for(auto& child : Children)
        {
            if(child->Bounds.Contains(p))
            {
                child->Add(p);
                return;
            }
        }
    }
    // Reached max particles, split this node
    else if(Particles.size() >= MaxParticles)
    {
        Split();
        Add(p);
    }
    else
    {
        Particles.push_back(p);
    }
}

void Octree::RenderDebug(DirectX::GeometricPrimitive* cube, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
    if(Particles.size() > 0)
    {
        auto avg  = (Bounds.TopLeft + Bounds.BottomRight) / 2;
        auto pos  = DirectX::SimpleMath::Vector3(avg.x, avg.y, avg.z);
        auto size = (Bounds.BottomRight.x - Bounds.TopLeft.x);

        DirectX::SimpleMath::Matrix world = DirectX::SimpleMath::Matrix::CreateScale(size) *
                                            DirectX::SimpleMath::Matrix::CreateTranslation(pos);

        cube->Draw(world, view, proj, DirectX::Colors::White, nullptr, true);
    }

    if(!IsLeaf)
    {
        for(auto& child : Children)
            child->RenderDebug(cube, view, proj);
    }
}