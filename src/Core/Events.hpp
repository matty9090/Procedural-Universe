#pragma once

#include "Event.hpp"
#include "Sim/INBodySim.hpp"

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