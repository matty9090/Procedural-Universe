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
    
};