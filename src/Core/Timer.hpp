#pragma once

#include <ctime>
#include <chrono>
#include <string>
#include <sstream>

#include "Services/Log.hpp"

class CTimer
{
public:
    CTimer(std::string name)
        : Name(name),
          Start(std::chrono::high_resolution_clock::now())
    {}
    
    ~CTimer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto span = std::chrono::duration_cast<std::chrono::duration<double>>(end - Start);
        auto val = round(span.count() * 1000000.0) / 1000.0;

        std::ostringstream ss;
        ss.precision(2);
        ss << std::fixed;
        ss << "Timer " << Name << " took " << val << "ms";

        auto str = ss.str();

        if (Verbose)
        {
            LOGV(str)
        }
        else
        {
            LOGM(str)
        }
    }

private:
    bool Verbose = false;
    std::string Name;
    std::chrono::high_resolution_clock::time_point Start;
};