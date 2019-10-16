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
    UseSplattingChanged,
    BHThetaChanged
};

typedef std::function<void(const EventData&)> EventCallback;

class EventStream
{
public:
    static void Report(EEvent event, EventData& data);
    static void Register(EEvent event, EventCallback callback);
    static void UnregisterAll(EEvent event);

private:
    static std::unordered_map<EEvent, std::list<EventCallback>> Events;
};