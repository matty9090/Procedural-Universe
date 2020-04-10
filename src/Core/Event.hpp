#pragma once

#include <list>
#include <functional>
#include <unordered_map>

#include "Events.hpp"

enum class EEvent
{
    SimSpeedChanged,
    NumParticlesChanged,
    IsPausedChanged,
    SimTypeChanged,
    GaussianBlurChanged,
    BloomBaseChanged,
    BloomAmountChanged,
    BloomSatChanged,
    BloomBaseSatChanged,
    SeederChanged,
    ForceFrame,
    RunBenchmark,
    BenchmarkResult,
    DrawDebugChanged,
    TrackParticle,
    LoadParticleFile,
    BHThetaChanged,
    UseBloomChanged,
    UseSplattingChanged,
    SandboxBloomBaseChanged
};

typedef std::function<void(const EventData&)> EventCallback;

template <class T>
auto EventValue(const EventData& data) -> decltype(static_cast<const T&>(data).Value)
{
    return static_cast<const T&>(data).Value;
}

class EventStream
{
public:
    static void Report(EEvent event, EventData& data);
    static void Register(EEvent event, EventCallback callback);
    static void UnregisterAll(EEvent event);

private:
    static std::unordered_map<EEvent, std::list<EventCallback>> Events;
};