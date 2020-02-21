#pragma once

#include <map>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

#include "Quadtree.hpp"
#include "Render/DX/ConstantBuffer.hpp"

using namespace DirectX::SimpleMath;

class CPlanet;

struct TerrainBuffer
{
    Matrix WorldViewProj;
    Matrix World;
};

struct TerrainPSBuffer
{
	Vector3 LightDir;
	float Custom;
};

struct TerrainVertex
{
    Vector3 Position;
    Vector3 Normal;
    Vector2 UV;
};

template <class HeightFunc>
class CTerrainNode : public Quadtree<CTerrainNode<HeightFunc>>
{
	public:
		CTerrainNode(CPlanet* planet, CTerrainNode* parent, EQuad quad = (EQuad)0);

        void Generate();
		void Update(float dt);
		void Render(Matrix viewProj);

		std::vector<UINT> GetEdge(EDir dir) { return Edges[dir]; }
		TerrainVertex GetVertex(int index) const { return Vertices[index]; }

        Matrix World;
		CPlanet* Planet;
		Quaternion Orientation;
		float Diameter = 0.0f;

	private:
		void NotifyNeighbours();
		void FixEdges();
		void FixEdge(EDir dir, CTerrainNode* neighbour, std::vector<UINT> edge, int depth);

		void SplitFunction() override;
		void MergeFunction() override;
		bool DistanceFunction() override;
		void TickFunction(float dt, int child) override { ChildNodes[child]->Update(dt); }

        Vector3 GetCenterWorld();
        Vector3 PointToSphere(Vector3 p);

        std::vector<TerrainVertex> Vertices;
        std::vector<UINT> Indices;
		std::map<EDir, std::vector<UINT>> Edges;

		bool Visible = true;
		HeightFunc GetHeight;

        ConstantBuffer<TerrainBuffer> Buffer;
        ConstantBuffer<TerrainPSBuffer> PSBuffer;

        Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
};

#include "TerrainNode.cpp"