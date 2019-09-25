#include "INBodySim.hpp"

#include "BarnesHut.hpp"
#include "BruteForce.hpp"

std::unique_ptr<INBodySim> CreateNBodySim(ID3D11DeviceContext* context, ENBodySim type)
{
    std::unique_ptr<INBodySim> sim;

    switch(type)
    {
        case ENBodySim::BruteForce:
            sim = std::make_unique<BruteForce>(context);
            break;

        case ENBodySim::BarnesHut:
            sim = std::make_unique<BarnesHut>();
            break;
    }

    return std::move(sim);
}