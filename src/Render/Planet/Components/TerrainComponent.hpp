#pragma once

#include <map>
#include <array>
#include <vector>
#include <memory>
#include <CommonStates.h>

#include "Render/DX/RenderCommon.hpp"
#include "PlanetComponent.hpp"

class CPlanet;
template <class HeightFunc> class CTerrainNode;

enum class EFace
{
    Top,
    Bottom,
    Left,
    Right,
    Front,
    Back
};

template <class HeightFunc>
class CTerrainComponent : public IPlanetComponent
{
    using FTerrainNode = CTerrainNode<HeightFunc>;

public:
    CTerrainComponent(CPlanet* planet);
    ~CTerrainComponent();

    static void GeneratePermutations();

    void Update(float dt) final;
    void Render(DirectX::SimpleMath::Matrix viewProj) final;

    enum EPermutations
    {
        Top = (1 << 0),
        Right = (1 << 1),
        Bottom = (1 << 2),
        Left = (1 << 3)
    };

    static UINT GridSize;
    static std::map<UINT, std::vector<UINT>> IndexPerm;

private:
    CPlanet* Planet;

    RenderPipeline TerrainPipeline;
    std::array<FTerrainNode*, 6> Nodes;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

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
};

#include "TerrainComponent.cpp"