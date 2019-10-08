#include "Octree.hpp"
#include "Services/Log.hpp"
#include "Sim/Physics.hpp"

Octree::Octree(const Cube& bounds, int depth)
    : Bounds(bounds),
      Size(bounds.BottomRight.x - bounds.TopLeft.x),
      Depth(depth)
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

                Children[i] = std::make_unique<Octree>(bounds, Depth + 1);

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
    if(NumParticles > 1)
    {
        for(auto& child : Children)
            if(child->Bounds.Contains(p))
                child->Add(p);
    }
    // Particle occupied, split this node and move previous particle
    else if(NumParticles == 1)
    {
        if(IsLeaf)
            Split();

        for(auto& child : Children)
        {
            if(child->Bounds.Contains(p)) child->Add(p);
            if(child->Bounds.Contains(P)) child->Add(P);
        }

        P = nullptr;
    }
    else
    {
        P = p;
    }

    ++NumParticles;
}

void Octree::CalculateMass()
{
    if(NumParticles == 1)
    {
        CentreOfMass = P->Position;
        TotalMass = P->Mass;
    }
    else if(!IsLeaf)
    {
        for(auto& child : Children)
        {
            child->CalculateMass();
            TotalMass += child->TotalMass;
            CentreOfMass += child->CentreOfMass * static_cast<float>(child->TotalMass);
        }

        if(TotalMass > 0.0)
            CentreOfMass /= static_cast<float>(TotalMass);
    }
}

Vec3d Octree::CalculateForce(Particle* p)
{
    Vec3d force;

    if(NumParticles == 1)
    {
        if(p != P && !Bounds.Contains(p))
        {
            auto f = Phys::Gravity(*p, *P);
            auto diff = p->Position - P->Position;
            diff.Normalize();

            force = Vec3d(f * diff.x, f * diff.y, f * diff.z);
        }
    }
    else
    {
        float r = (p->Position - CentreOfMass).Length();
        float d = Bounds.BottomRight.x - Bounds.TopLeft.x;

        if(d / r < Theta)
        {
            auto f = Phys::Gravity(*p, CentreOfMass, TotalMass);
            auto diff = p->Position - CentreOfMass;
            diff.Normalize();

            force = Vec3d(f * diff.x, f * diff.y, f * diff.z);
        }
        else if(!IsLeaf)
        {
            for(auto& child : Children)
            {
                force += child->CalculateForce(p);
            }
        }
    }

    return force;
}

void Octree::RenderDebug(DirectX::GeometricPrimitive* cube, DirectX::GeometricPrimitive* sphere, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
    if(Depth > 0 && !IsLeaf)
    {
        auto avg  = (Bounds.TopLeft + Bounds.BottomRight) / 2;
        auto pos  = DirectX::SimpleMath::Vector3(avg.x, avg.y, avg.z);
        auto size = (Bounds.BottomRight.x - Bounds.TopLeft.x);

        DirectX::SimpleMath::Matrix cworld = DirectX::SimpleMath::Matrix::CreateScale(size) *
                                             DirectX::SimpleMath::Matrix::CreateTranslation(pos);

        DirectX::SimpleMath::Matrix sworld = DirectX::SimpleMath::Matrix::CreateScale(200.0f / static_cast<float>(Depth)) *
                                             DirectX::SimpleMath::Matrix::CreateTranslation(CentreOfMass);    

        auto colp = pos;
        colp.Normalize();

        auto col = DirectX::SimpleMath::Color(colp.x, colp.y, colp.z);

        cube->Draw(cworld, view, proj, col, nullptr, true);
        // sphere->Draw(sworld, view, proj, DirectX::Colors::Blue);
    }

    if(!IsLeaf)
    {
        for(auto& child : Children)
            child->RenderDebug(cube, sphere, view, proj);
    }
}