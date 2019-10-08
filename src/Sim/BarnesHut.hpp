#pragma once

#include <GeometricPrimitive.h>

#include "Octree.hpp"
#include "INBodySim.hpp"

class BarnesHut : public INBodySim
{
    public:
        BarnesHut(ID3D11DeviceContext* context);

        void Init(std::vector<Particle>& particles) final;
        void Update(float dt) final;
        void RenderDebug(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

    private:
        Cube Bounds;

        std::unique_ptr<Octree> Tree;
        std::vector<Particle>* Particles;

        ID3D11DeviceContext* Context;
        
        std::unique_ptr<DirectX::GeometricPrimitive> DebugCube;
        std::unique_ptr<DirectX::GeometricPrimitive> DebugSphere;
};