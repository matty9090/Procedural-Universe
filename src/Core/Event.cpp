#include "Event.hpp"

std::unordered_map<EEvent, std::list<EventCallback>> EventStream::Events;

void EventStream::Register(EEvent event, EventCallback callback)
{
    Events[event].push_back(callback);
}

void EventStream::Report(EEvent event, EventData& data)
{
    for(auto callback : Events[event])
        callback(data);
}

void EventStream::UnregisterAll(EEvent event)
{
    Events[event].clear();
}