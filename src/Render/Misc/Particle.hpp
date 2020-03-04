#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "Core/Vec3.hpp"

struct Particle
{
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Color Colour;
    DirectX::SimpleMath::Color OriginalColour;

    Vec3d Velocity;
    Vec3d Forces;
    
    double Mass;
};

struct LWParticle
{
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Color Colour;
    float Scale;
};

#define define_has_member(member_name)                                         \
    template <typename T>                                                      \
    class has_member_##member_name                                             \
    {                                                                          \
        typedef char yes_type;                                                 \
        typedef long no_type;                                                  \
        template <typename U> static yes_type test(decltype(&U::member_name)); \
        template <typename U> static no_type  test(...);                       \
    public:                                                                    \
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_type);  \
    }

define_has_member(Velocity);
define_has_member(Mass);
define_has_member(Colour);
define_has_member(OriginalColour);
define_has_member(Forces);
define_has_member(Scale);

#define has_member(class_, member_name) has_member_##member_name<class_>::value

template <class T, typename std::enable_if<has_member(T, Velocity), T>::type * = nullptr>
void AddParticleVelocity(T& particle, Vec3d v) { particle.Velocity = v; }

template <class T, typename std::enable_if<!has_member(T, Velocity), T>::type * = nullptr>
void AddParticleVelocity(T& particle, Vec3d v) {}

template <class T, typename std::enable_if<has_member(T, Mass), T>::type * = nullptr>
void AddParticleMass(T& particle, double v) { particle.Mass = v; }

template <class T, typename std::enable_if<!has_member(T, Mass), T>::type * = nullptr>
void AddParticleMass(T& particle, double v) {}

template <class T, typename std::enable_if<has_member(T, Colour), T>::type * = nullptr>
void AddParticleColour(T& particle, DirectX::SimpleMath::Color v) { particle.Colour = v; }

template <class T, typename std::enable_if<!has_member(T, Colour), T>::type * = nullptr>
void AddParticleColour(T& particle, DirectX::SimpleMath::Color v) {}

template <class T, typename std::enable_if<has_member(T, OriginalColour), T>::type * = nullptr>
void AddParticleOriginalColour(T& particle, DirectX::SimpleMath::Color v) { particle.OriginalColour = v; }

template <class T, typename std::enable_if<!has_member(T, OriginalColour), T>::type * = nullptr>
void AddParticleOriginalColour(T& particle, DirectX::SimpleMath::Color v) {}

template <class T, typename std::enable_if<has_member(T, Forces), T>::type * = nullptr>
void AddParticleForces(T& particle, Vec3d v) { particle.Forces = v; }

template <class T, typename std::enable_if<!has_member(T, Forces), T>::type * = nullptr>
void AddParticleForces(T& particle, Vec3d v) {}

template <class T, typename std::enable_if<has_member(T, Scale), T>::type * = nullptr>
void AddParticleScale(T& particle, float v) { particle.Scale = v; }

template <class T, typename std::enable_if<!has_member(T, Scale), T>::type * = nullptr>
void AddParticleScale(T& particle, float v) {}