#pragma once

#include <d3d11.h>
#include <vector>
#include <map>

#include "Core/Event.hpp"
#include "Sim/INBodySim.hpp"
#include "Sim/IParticleSeeder.hpp"

class UI
{
public:
    UI(ID3D11DeviceContext* context, HWND hwnd);
    ~UI();

    void Render();
    void Update(float dt);
    void Reset();

    void SetSelectedParticle(Particle* particle);

private:
    int Frames = 0;
    float FPSUpdate = 0.5f;
    float FPS = 0.0f, FPSTimer = FPSUpdate;

    float GaussianBlur = 2.0f;
    float BloomAmount = 1.0f, BloomBase = 1.4f;

    bool Paused = false;
    float SimSpeed = 1.0f;
    int Particles = 100;
    int SelectedSeeder = 0;

    std::map<ENBodySim, BenchmarkEventData> BenchmarkData;

    Particle* SelectedParticle = nullptr;
    ENBodySim SimType = ENBodySim::BruteForceCPU;
    EParticleSeeder SeederType = EParticleSeeder::Random;
};