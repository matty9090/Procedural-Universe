#pragma once

#include <d3d11.h>
#include <vector>

#include "Sim/INBodySim.hpp"

class UI
{
public:
    UI(ID3D11DeviceContext* context, HWND hwnd);
    ~UI();

    void Render();
    void Update(float dt);
    void Reset();

    bool IsPaused() const { return Paused; }
    int GetNumParticles() const { return Particles; }
    ENBodySim GetSimType() const { return SimType; }

private:
    int Frames = 0;
    float FPSUpdate = 0.5f;
    float FPS = 0.0f, FPSTimer = FPSUpdate;

    bool Paused = false;
    int Particles = 1000;
    ENBodySim SimType = ENBodySim::BruteForce;
};