#pragma once

#include "Octree.hpp"
#include "INBodySim.hpp"

class BarnesHut : public INBodySim
{
    public:
        BarnesHut();

        void Init(std::vector<Particle>& particles) final {}
        void Update(float dt) final {}

    private:
        std::unique_ptr<Octree> Tree;
        std::vector<Particle>* Particles;
};