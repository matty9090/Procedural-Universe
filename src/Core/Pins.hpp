#pragma once

#include <vector>

enum class EPin
{
    Float,
    UV
};

struct SPin
{
    EPin Type;
    std::string Name;
    int Id = 0;
};

using PinList = std::vector<SPin>;