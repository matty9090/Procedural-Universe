#pragma once

#include <map>
#include <array>
#include <vector>
#include <memory>
#include <d3d11.h>
#include <SimpleMath.h>
#include <CommonStates.h>

#include "Render/DX/RenderCommon.hpp"
#include "Render/Cameras/Camera.hpp"
#include "Misc/FastNoise.h"

class CTerrainNode;

enum class EFace
{
    Top,
    Bottom,
    Left,
    Right,
    Front,
    Back
};

class CPlanet
{
public:
    CPlanet(ID3D11DeviceContext* context, ICamera* cam);
    ~CPlanet();

    static void GeneratePermutations();

    void Update(float dt);
    void Render();
    void Move(DirectX::SimpleMath::Vector3 v);
    void SetPosition(DirectX::SimpleMath::Vector3 p);
    float GetHeight(DirectX::SimpleMath::Vector3 normal);

    ID3D11Device* GetDevice() const { return Device; }
    ID3D11DeviceContext* GetContext() const { return Context; }

    static UINT GridSize;
    
    ICamera* Camera;
    float Radius = 1000.0f;
    float SplitDistance = 1500.0f;
    DirectX::SimpleMath::Matrix World;

    // Terrain
    std::array<CTerrainNode*, 6> Nodes;
    RenderPipeline TerrainPipeline;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    static std::map<UINT, std::vector<UINT>> IndexPerm;
    FastNoise Noise;

    std::map<EFace, DirectX::SimpleMath::Vector3> Orientations = {
        { EFace::Top,    {  90.0f,   0.0f, 0.0f } },
        { EFace::Bottom, { -90.0f,   0.0f, 0.0f } },
        { EFace::Left,   {   0.0f,  90.0f, 0.0f } },
        { EFace::Right,  {   0.0f, -90.0f, 0.0f } },
        { EFace::Front,  {   0.0f,   0.0f, 0.0f } },
        { EFace::Back,   {   0.0f, 180.0f, 0.0f } }
    };

    std::map<EFace, std::array<EFace, 4>> Neighbours = {
        { EFace::Top,    { EFace::Front, EFace::Left,  EFace::Back,   EFace::Right } },
        { EFace::Bottom, { EFace::Back , EFace::Left,  EFace::Front,  EFace::Right } },
        { EFace::Left,   { EFace::Top  , EFace::Front, EFace::Bottom, EFace::Back  } },
        { EFace::Right,  { EFace::Top  , EFace::Back,  EFace::Bottom, EFace::Front } },
        { EFace::Front,  { EFace::Top  , EFace::Right, EFace::Bottom, EFace::Left  } },
        { EFace::Back,   { EFace::Top  , EFace::Left,  EFace::Bottom, EFace::Right } }
    };

    enum EPermutations
    {
        Top = (1 << 0),
        Right = (1 << 1),
        Bottom = (1 << 2),
        Left = (1 << 3)
    };

private:
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;

    DirectX::SimpleMath::Vector3 Position;

    void UpdateMatrix();
};