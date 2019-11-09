#pragma once

#include <map>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

#include "Render/Cameras/Camera.hpp"

class CTerrainNode;

class CPlanet
{
public:
    CPlanet(ID3D11DeviceContext* context, ICamera* cam);

    void Update(float dt);
    float GetHeight(DirectX::SimpleMath::Vector3 normal);

    static UINT GridSize;

    ICamera* Camera;
    float Radius = 1000.0f;
    float SplitDistance = 50.0f;
    DirectX::SimpleMath::Matrix World;

    static std::map<UINT, std::vector<UINT>> IndexPerm;

    enum EPermutations
    {
        Top = (1 << 0),
        Right = (1 << 1),
        Bottom = (1 << 2),
        Left = (1 << 3)
    };

private:
    static void GeneratePermutations();

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
};