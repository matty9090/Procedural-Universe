#include "INBodySim.hpp"

#include "BarnesHut.hpp"
#include "BruteForce.hpp"

std::unique_ptr<INBodySim> CreateNBodySim(ID3D11DeviceContext* context, std::vector<Particle>& particles, ENBodySim type)
{
    std::unique_ptr<INBodySim> sim;

    switch(type)
    {
        case ENBodySim::BruteForce:
            sim = std::make_unique<BruteForce>(context, particles);
            break;

        case ENBodySim::BarnesHut:
            sim = std::make_unique<BarnesHut>(particles);
            break;
    }

    return std::move(sim);
}