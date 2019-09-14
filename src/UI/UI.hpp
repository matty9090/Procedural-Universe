#pragma once

#include <d3d11.h>
#include <vector>

class UI
{
public:
    UI(ID3D11DeviceContext* context, HWND hwnd);
    ~UI();

    void Render();
    void Update(float dt);
    void Reset();

private:
    int Frames = 0;
    float FPSUpdate = 0.5f;
    float FPS = 0.0f, FPSTimer = FPSUpdate;

    int Particles = 1000;
};