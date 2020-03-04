#include "Maths.hpp"

float Maths::RandFloat()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

float Maths::RandFloat(float min, float max)
{
    return (static_cast<float>(rand()) / RAND_MAX)* (max - min) + min;
}