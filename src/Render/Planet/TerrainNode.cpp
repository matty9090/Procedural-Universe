#include "TerrainNode.hpp"
#include "Planet.hpp"

#include <string>
#include <cassert>
#include <exception>


CTerrainNode::CTerrainNode(CPlanet* planet, EQuad quad, CTerrainNode* parent)
	: Quadtree(quad, parent),
      Planet(planet)
{
    for (size_t child = 0; child < 4; ++child) {
        ChildNodes[child] = nullptr;
    }

    if (Parent != nullptr)
    {
        Depth = Parent->GetDepth() + 1;
        World = Parent->World;
        Diameter = Bounds.size * Planet->Radius * 2;
        Orientation = Parent->Orientation;
    }
}

void CTerrainNode::Generate()
{
	ClearMeshData();

	int gridsize = Planet->GridSize, gh = Planet->GridSize / 2;

    Tangents.reserve(gridsize * gridsize);
    Vertices.reserve(gridsize * gridsize);

	float step = Bounds.size / (gridsize - 1);
	int k = 0, sx = 0, sy = 0;

	switch (Quad)
	{
		case NW: sx = 0,  sy = 0; break;
		case NE: sx = gh, sy = 0; break;
		case SE: sx = gh, sy = gh; break;
		case SW: sx = 0,  sy = gh; break;
	}

	for (int y = 0; y < gridsize; ++y)
	{
		float yy = Bounds.y + y * step;

		for (int x = 0; x < gridsize; ++x, ++k)
		{
			if (Parent && (x % 2 == 0) && (y % 2 == 0))
			{
				int xh = sx + x / 2;
				int yh = sy + y / 2;

				Vertices.push_back(Parent->GetVertex(xh + yh * gridsize));
				Normals.push_back(Parent->GetNormal(xh + yh * gridsize));
			}
			else
			{
				float xx = Bounds.x + x * step;

				Vector3 pos = Vector3(xx, yy, 1.0f);
				pos = Vector3::Transform(PointToSphere(pos), Orientation);;

				Vector3 normal = pos;
				normal.Normalize();

				float height = Planet->GetHeight(normal);
				Vector3 finalPos = pos * Planet->Radius + normal * height;

				Vertices.push_back(finalPos);
				Normals.push_back(normal);

				Vector3 morphPos = finalPos, morphNorm = normal;
			}
		}
	}

	Triangles = Planet->IndexPerm[0];
    // TODO: Generate

	float morph = Bounds.size * Planet->SplitDistance;
}

void CTerrainNode::ClearMeshData()
{
	
}

void CTerrainNode::NotifyNeighbours()
{
	std::vector<CTerrainNode*> neighbours;

    for (int i = 0; i < 4; ++i)
    {
        auto sn = GetSmallerNeighbours(GetGreaterThanOrEqualNeighbour(i), i);
		neighbours.insert(neighbours.end(), sn.begin(), sn.end());
    }

	for (auto n : neighbours)
		n->FixEdges();
}

void CTerrainNode::FixEdges()
{
	std::vector<CTerrainNode*> neighbours(4);
	std::vector<bool> depths(4);

	for (int i = 0; i < 4; ++i) neighbours[i] = GetGreaterThanOrEqualNeighbour(i);
	for (int i = 0; i < 4; ++i) depths[i]     = neighbours[i] && (Depth - neighbours[i]->GetDepth() >= 1);

	int d0 = depths[North] ? CPlanet::Top    : 0;
	int d1 = depths[East]  ? CPlanet::Right  : 0;
	int d2 = depths[South] ? CPlanet::Bottom : 0;
	int d3 = depths[West]  ? CPlanet::Left   : 0;

	int perm = d0 | d1 | d2 | d3;
	Triangles = Planet->IndexPerm[perm];

    // TODO: Generate
}

Vector3 CTerrainNode::PointToSphere(Vector3 p)
{
    float x2 = p.x * p.x, y2 = p.y * p.y, z2 = p.z * p.z;

	return Vector3(p.x * sqrtf(1.0f - y2 * 0.5f - z2 * 0.5f + (y2 * z2) * 0.33333333f),
		           p.y * sqrtf(1.0f - z2 * 0.5f - x2 * 0.5f + (z2 * x2) * 0.33333333f),
		           p.z * sqrtf(1.0f - x2 * 0.5f - y2 * 0.5f + (x2 * y2) * 0.33333333f));
}

void CTerrainNode::Update(float dt)
{
	Quadtree::Update(dt);
}

Vector3 CTerrainNode::GetCenterWorld()
{
    Vector3 midpoint = Vertices[(Planet->GridSize * Planet->GridSize) / 2];
    return Vector3::Transform(midpoint, World);
}


void CTerrainNode::SplitFunction()
{
	float x = Bounds.x, y = Bounds.y;
	float d = Bounds.size / 2;

	for (int i = 0; i < 4; ++i)
	{
        ChildNodes[i] = new CTerrainNode(Planet, static_cast<EQuad>(i), this);
	}

	ChildNodes[NW]->SetBounds(Square{ x    , y    , d });
	ChildNodes[NE]->SetBounds(Square{ x + d, y    , d });
	ChildNodes[SE]->SetBounds(Square{ x + d, y + d, d });
	ChildNodes[SW]->SetBounds(Square{ x    , y + d, d });

	for (int i = 0; i < 4; ++i)
		ChildNodes[i]->Generate();

	for (int i = 0; i < 4; ++i)
		ChildNodes[i]->FixEdges();

	HideFunction();
	NotifyNeighbours();
}

void CTerrainNode::MergeFunction()
{
	for (int i = 0; i < 4; ++i)
	{
		ChildNodes[i] = nullptr;
	}

	NotifyNeighbours();
}

void CTerrainNode::HideFunction()
{

}

bool CTerrainNode::DistanceFunction()
{
	Vector3 cam = Planet->Camera->GetPosition();
	float distance = Vector3::Distance(cam, GetCenterWorld());

	//float height = Vector3::Distance(cam, Planet->GetTransform().GetLocation()) - Radius * 0.96f;
	//float horizon = sqrtf(height * (2 * Radius + height));
	//bool visible = distance - Diameter < horizon;

	//SetActorHiddenInGame(!visible);

	return Depth < 8 && distance < Bounds.size * Planet->SplitDistance;
}