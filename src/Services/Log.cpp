#include "Log.hpp"
#include <iostream>

#include <d3d11.h>
#include <SimpleMath.h>

void FLog::Log(int num, ELogType logLevel)
{
#ifdef _DEBUG
    std::cout << "[" << LogTypes[logLevel] << "] " << num << std::endl;
#else
    if (logLevel != Verbose)
        std::cout << "[" << LogTypes[logLevel] << "] " << num << std::endl;
#endif
}

void FLog::Log(double num, ELogType logLevel)
{
#ifdef _DEBUG
    std::cout << "[" << LogTypes[logLevel] << "] " << num << std::endl;
#else
    if (logLevel != Verbose)
        std::cout << "[" << LogTypes[logLevel] << "] " << num << std::endl;
#endif
}


void FLog::Log(std::string msg, ELogType logLevel)
{
#ifdef _DEBUG
    std::cout << "[" << LogTypes[logLevel] << "] " << msg << std::endl;
#else
    if(logLevel != Verbose)
        std::cout << "[" << LogTypes[logLevel] << "] " << msg << std::endl;
#endif
}

void FLog::Log(Vec3d v, ELogType logLevel)
{
#ifdef _DEBUG
    std::cout << "[" << LogTypes[logLevel] << "] (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
#else
    if (logLevel != Verbose)
        std::cout << "[" << LogTypes[logLevel] << "] (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
#endif
}

void FLog::Log(DirectX::SimpleMath::Vector2 v, ELogType logLevel)
{
#ifdef _DEBUG
    std::cout << "[" << LogTypes[logLevel] << "] (" << v.x << ", " << v.y << ")" << std::endl;
#else
    if (logLevel != Verbose)
        std::cout << "[" << LogTypes[logLevel] << "] (" << v.x << ", " << v.y << ")" << std::endl;
#endif
}

void FLog::Log(DirectX::SimpleMath::Vector3 v, ELogType logLevel)
{
#ifdef _DEBUG
    std::cout << "[" << LogTypes[logLevel] << "] (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
#else
    if (logLevel != Verbose)
        std::cout << "[" << LogTypes[logLevel] << "] (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
#endif
}

void FLog::Log(DirectX::SimpleMath::Color v, ELogType logLevel)
{
#ifdef _DEBUG
    std::cout << "[" << LogTypes[logLevel] << "] (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
#else
    if (logLevel != Verbose)
        std::cout << "[" << LogTypes[logLevel] << "] (" << v.R() << ", " << v.G() << ", " << v.B() << ", " << v.A() << ")" << std::endl;
#endif
}