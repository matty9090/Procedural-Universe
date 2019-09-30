#include "BarnesHut.hpp"
#include "Services/Log.hpp"

BarnesHut::BarnesHut()
{
    FLog::Get().Log("Barnes-Hut");

    const float size = 1000.0f;

    Cube bounds = {
        { -size, -size, -size },
        { +size, +size, +size }
    };

    Tree = std::make_unique<Octree>(bounds);
}