#include "Maths.hpp"

int Maths::RandInt(int min, int max)
{
    return (rand() % (max - min)) + min;
}

float Maths::RandFloat()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

float Maths::RandFloat(float min, float max)
{
    return (static_cast<float>(rand()) / RAND_MAX) * (max - min) + min;
}

bool closeEnough(const float& a, const float& b, const float& epsilon = std::numeric_limits<float>::epsilon()) {
    return (epsilon > std::abs(a - b));
}
