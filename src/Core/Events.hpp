#pragma once

#include "Event.hpp"

#include "Sim/INBodySim.hpp"
#include "Sim/IParticleSeeder.hpp"

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

struct SimTypeEventData : public EventData
{
    ENBodySim Value;

    SimTypeEventData(ENBodySim value) : Value(value) {}
};

struct SeederTypeEventData : public EventData
{
    EParticleSeeder Value;

    SeederTypeEventData(EParticleSeeder value) : Value(value) {}
};

struct BenchmarkEventData : public EventData
{
    ENBodySim SimType;
    int Time;

    BenchmarkEventData() : Time(0) {}
    BenchmarkEventData(ENBodySim sim, int t) : SimType(sim), Time(t) {}
};