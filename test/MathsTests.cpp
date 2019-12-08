#include "gtest/gtest.h"
#include "Core/Maths.hpp"

TEST(IndependentMethod, ClosestParticle1)
{
    std::vector<Particle> particles = {
        {{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f, 0.0f, 5.0f }},
        {{ 2.0f, 7.0f, 0.0f }},
        {{ 3.0f, 16.0f, 0.0f }},
        {{ 60.0f, 0.0f, 10.0f }}
    };

    size_t ID;
    auto p = Maths::ClosestParticle(DirectX::SimpleMath::Vector3(1.0f, 7.0f, 1.0f), particles, &ID);

    ASSERT_EQ(ID, 2U) << "Wrong closest particle";
}

TEST(IndependentMethod, ClosestParticle2)
{
    std::vector<Particle> particles = {
        {{ 0.0f, 0.0f, 0.0f }},
        {{ 1.0f, 0.0f, 5.0f }},
        {{ 2.0f, 7.0f, 0.0f }},
        {{ 3.0f, 16.0f, 0.0f }},
        {{ 60.0f, 0.0f, 10.0f }}
    };

    size_t ID;
    auto p = Maths::ClosestParticle(DirectX::SimpleMath::Vector3(50.0f, 2.0f, 7.0f), particles, &ID);

    ASSERT_EQ(ID, 4U) << "Wrong closest particle";
}