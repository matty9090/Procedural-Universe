#pragma once

#include "Quadtree.hpp"

#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class CPlanet;

enum class EFace
{
    Top,
    Bottom,
    Left,
    Right,
    Front,
    Back
};

class CTerrainNode : public Quadtree<CTerrainNode>
{
	public:	
		CTerrainNode(CPlanet* planet, EQuad quad, CTerrainNode* parent);

		void Update(float dt);

		Vector3 GetVertex(int index) const { return Vertices[index]; }
        Vector3 GetNormal(int index) const { return Normals[index]; }

        Matrix World;
		CPlanet* Planet;
		Quaternion Orientation;

	private:
		void Generate();
		void ClearMeshData();
		void NotifyNeighbours();
		void FixEdges();

		void SplitFunction() override;
		void MergeFunction() override;
        void HideFunction()  override;
		bool DistanceFunction() override;
		void TickFunction(float dt, int child) override { ChildNodes[child]->Update(dt); }

        Vector3 GetCenterWorld();
        Vector3 PointToSphere(Vector3 p);

		float Diameter = 0.0f;

		std::vector<Vector3> Vertices;
		std::vector<Vector3> Normals;
		std::vector<UINT>    Triangles;
		std::vector<Vector2> UV;
		std::vector<Vector3> Tangents;
};
