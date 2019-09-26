#include "INBodySim.hpp"

#include "BarnesHut.hpp"
#include "BruteForceCPU.hpp"
#include "BruteForceGPU.hpp"

std::unique_ptr<INBodySim> CreateNBodySim(ID3D11DeviceContext* context, ENBodySim type)
{
    std::unique_ptr<INBodySim> sim;

    switch(type)
    {
        case ENBodySim::BruteForceCPU:
            sim = std::make_unique<BruteForceCPU>(context);
            break;

        case ENBodySim::BruteForceGPU:
            sim = std::make_unique<BruteForceGPU>(context);
            break;

        case ENBodySim::BarnesHut:
            sim = std::make_unique<BarnesHut>();
            break;
    }

    return std::move(sim);
}

std::string NBodySimGetName(ENBodySim type)
{
    switch(type)
    {
        case ENBodySim::BruteForceCPU:  return "Brute Force CPU"; break;
        case ENBodySim::BruteForceGPU:  return "Brute Force GPU"; break;
        case ENBodySim::BarnesHut:      return "Barnes-Hut"; break;
    }

    return "Unknown";
}