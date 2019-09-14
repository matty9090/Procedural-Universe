#pragma once

#include <map>
#include <string>

namespace DirectX
{
    namespace SimpleMath
    {
        struct Vector2;
        struct Vector3;
    }
}

class FLog
{
public:
    enum ELogType { Verbose, Info, Warning, Error };

    static FLog& Get()
    {
        static FLog instance;
        return instance;
    }

    FLog(FLog const&) = delete;
    void operator=(FLog const&) = delete;

    void Log(int num, ELogType logLevel = Info);
    void Log(double num, ELogType logLevel = Info);
    void Log(std::string msg, ELogType logLevel = Info);
    void Log(DirectX::SimpleMath::Vector2 v, ELogType logLevel = Info);
    void Log(DirectX::SimpleMath::Vector3 v, ELogType logLevel = Info);

private:
    FLog() {}

    std::map<ELogType, std::string> LogTypes = {
        { Verbose, "Verbose"},
        { Info, "Info"},
        { Warning, "Warning"},
        { Error, "Error"}
    };
};
