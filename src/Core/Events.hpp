#pragma once

#include "Event.hpp"

#include "Sim/INBodySim.hpp"
#include "Sim/IParticleSeeder.hpp"
#include "Render/Misc/Particle.hpp"

struct EventData {};

struct IntEventData : public EventData
{
    int Value;

    IntEventData(int value) : Value(value) {}
};

struct FloatEventData : public EventData
{
    float Value;

    FloatEventData(float value) : Value(value) {}
};

struct BoolEventData : public EventData
{
    bool Value;

    BoolEventData(bool value) : Value(value) {}
};

struct StringEventData : public EventData
{
    std::string Value;

    StringEventData(std::string value) : Value(value) {}
};

struct SimTypeEventData : public EventData
{
    ENBodySim Value;

    SimTypeEventData(ENBodySim value) : Value(value) {}
};

struct SeederTypeEventData : public EventData
{
    uint8_t Value;

    SeederTypeEventData(uint8_t value) : Value(value) {}
};

struct BenchmarkEventData : public EventData
{
    ENBodySim SimType;
    int Time;

    BenchmarkEventData() : Time(0) {}
    BenchmarkEventData(ENBodySim sim, int t) : SimType(sim), Time(t) {}
};

struct ParticleEventData : public EventData
{
    Particle* Value;

    ParticleEventData(Particle* value) : Value(value) {}
};