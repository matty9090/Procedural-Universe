#pragma once

#include <GeometricPrimitive.h>

#include "Octree.hpp"
#include "INBodySim.hpp"
#include "Render/Model/Cube.hpp"
#include "Core/ThreadPool.hpp"

class BarnesHut : public INBodySim
{
    public:
        BarnesHut(ID3D11DeviceContext* context);
        ~BarnesHut();

        void Init(std::vector<Particle>& particles) final;
        void Update(float dt) final;
        void RenderDebug(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

    private:
        BoundingCube Bounds;

        std::unique_ptr<Octree> Tree;
        std::vector<Particle>* Particles;

        ID3D11DeviceContext* Context;

        struct ParticleInfo
        {
            size_t Index;
            size_t Loops;
        };

        CThreadPool<ParticleInfo> Pool;
        
        std::unique_ptr<Cube> DebugCube;
        std::unique_ptr<DirectX::GeometricPrimitive> DebugSphere;

        void Exec(const ParticleInfo& info);
};