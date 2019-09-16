#pragma once

#include <list>
#include <functional>
#include <unordered_map>

struct EventData {};

enum class EEvent {
    UpdateSimSpeed
};

typedef std::function<void(const EventData&)> EventCallback;

class EventStream
{
public:
    static void Report(EEvent event, EventData& data);
    static void Register(EEvent event, EventCallback callback);

private:
    static std::unordered_map<EEvent, std::list<EventCallback>> Events;
};