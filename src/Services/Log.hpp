#pragma once

#include <map>
#include <string>
#include <locale>
#include <codecvt>

#include "Core/Vec3.hpp"

#define LOGV(str) FLog::Get().Log(str, FLog::Verbose);
#define LOGM(str) FLog::Get().Log(str, FLog::Info);
#define LOGW(str) FLog::Get().Log(str, FLog::Warning);
#define LOGE(str) FLog::Get().Log(str, FLog::Error);

inline std::string wstrtostr(std::wstring str)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(str);
}

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
    void Log(float num, ELogType logLevel = Info);
    void Log(double num, ELogType logLevel = Info);
    void Log(std::string msg, ELogType logLevel = Info);
    void Log(Vec3d v, ELogType logLevel = Info);
    void Log(DirectX::SimpleMath::Vector2 v, ELogType logLevel = Info);
    void Log(DirectX::SimpleMath::Vector3 v, ELogType logLevel = Info);
    void Log(DirectX::SimpleMath::Color v, ELogType logLevel = Info);

private:
    FLog() {}

    std::map<ELogType, std::string> LogTypes = {
        { Verbose, "Verbose"},
        { Info, "Info"},
        { Warning, "Warning"},
        { Error, "Error"}
    };
};
